import sys, os
DEFAULT_VERS = "310"
SOURCE_DIR = "source"
HOOK_MAGIC = "// hook_from "

buildVersion = None
patchConfig = {
    "build_id" : {},
    "nso_load_addr" : {},
}

def initConfig():
    configPath = os.path.join(PATCH_CONFIG_DIR, buildVersion + PATCH_CONFIG_EXTENSION)
    # read config file
    with open(configPath) as configFile:
        curConfigName = None
        for line in configFile:
            line = line.strip()
            configNameLineMatch = re.match(r'\[(.+)\]', line)
            if configNameLineMatch:
                curConfigName = configNameLineMatch.group(1)
                continue

            if '=' in line:
                configNSO, configValue = line.split('=', 1)
                if not configNSO.isalnum():
                    continue
                if '+' in configValue:
                    print("genPatch.py error:", line, "awaits implementation")
                    sys.exit(-1)
                patchConfig[curConfigName][configNSO] = configValue


def calcJump(from_addr_str, dest_func, vers=DEFAULT_VERS):
    from_addr = int(from_addr_str, 16)
    dest_func = dest_func + "("

    mapFilePath = "build" + vers + "/skyline" + vers + ".map"
    with open(mapFilePath, 'r') as f:
        mapFile = f.read()

    foundPos = mapFile.find(dest_func) - 34
    foundLine = mapFile[foundPos:mapFile.find("\n", foundPos)]

    print("Found:")
    print(foundLine)

    func_addr = int(foundLine[:foundLine.find(" ")], 0)
    jump_offset = patchConfig["nso_load_addr"]["subsdk1"] + func_addr - from_addr
    print("Jump needed: " + hex(jump_offset))

initConfig()

if len(sys.argv) > 3:
    calcJump(sys.argv[1], sys.argv[2], sys.argv[3])
elif len(sys.argv) > 2:
    calcJump(sys.argv[1], sys.argv[2])
else:
    hasOutput = False
    for root, subdirs, files in os.walk(SOURCE_DIR):
        for file in files:
            with open(root+"/"+file, 'r') as f:
                file_iter = iter(f.readlines())
            for line in file_iter:
                if HOOK_MAGIC in line:
                    hook_addr = line[len(HOOK_MAGIC):-1]
                    line = next(file_iter)
                    hook_func = line[:line.find('(')]
                    hook_func = hook_func[hook_func.rfind(' ') + 1:]
                    calcJump(hook_addr, hook_func)
                    hasOutput = True
            
    if not hasOutput:
        print("Usage: %s [from addr] [to func name] (s2 vers, like '310')" % sys.argv[0])
