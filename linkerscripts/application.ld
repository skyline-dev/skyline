OUTPUT_FORMAT(elf64-littleaarch64)
OUTPUT_ARCH(aarch64)
ENTRY(__module_start)

PHDRS
{  
  text PT_LOAD FLAGS(5);
  rodata PT_LOAD FLAGS(4);
  data PT_LOAD FLAGS(6);
  dynamic PT_DYNAMIC;
}

SECTIONS
{
  PROVIDE(__start__ = 0x0);
  . = __start__;
  __code_start__ = .;

  /* App code */
  .text : {
    HIDDEN(__text_start__ = .);
    KEEP (*(.text.crt0))
    *(.text .text.*)
    HIDDEN(__text_end__ = .);
  } :text

  /* Trampoline and stuffs */
  .plt : { *(.plt .plt.*) } :text

  /* Read-only sections */
  . = ALIGN(0x1000);
	__rodata_start = . ;

  /* App name */
  .application_name : { KEEP (*(.rodata.application_name)) } :rodata

  /* Make sure everything is aligned */
  . = ALIGN(8);

  /* App rodata */
  .rodata : {
    *(.rodata .rodata.*)
  } :rodata

  /* All the symbols needed for relocation lookup */
  .hash     : { *(.hash) } :rodata
  .gnu.hash : { *(.gnu.hash) } :rodata
  .dynsym   : { *(.dynsym .dynsym.*) } :rodata
  .dynstr   : { *(.dynstr .dynstr.*) } :rodata

  /* All relocations sections */
  .rel.init       : { *(.rel.init) } :rodata
  .rela.init      : { *(.rela.init) } :rodata
  .rel.text       : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*) } :rodata
  .rela.text      : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) } :rodata
  .rel.fini       : { *(.rel.fini) } :rodata
  .rela.fini      : { *(.rela.fini) } :rodata
  .rel.rodata     : { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*) } :rodata
  .rela.rodata    : { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) } :rodata
  .rel.data       : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*) } :rodata
  .rela.data      : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) } :rodata
  .rel.tdata	    : { *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*) } :rodata
  .rela.tdata	    : { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) } :rodata
  .rel.tbss	      : { *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*) } :rodata
  .rela.tbss	    : { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) } :rodata
  .rel.ctors      : { *(.rel.ctors) } :rodata
  .rela.ctors     : { *(.rela.ctors) } :rodata
  .rel.dtors      : { *(.rel.dtors) } :rodata
  .rela.dtors     : { *(.rela.dtors) } :rodata
  .rel.got        : { *(.rel.got) } :rodata
  .rela.got       : { *(.rela.got) } :rodata
  .rel.bss        : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*) } :rodata
  .rela.bss       : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*) } :rodata
  .rel.plt        : { *(.rel.plt) } :rodata
  .rela.plt       : { *(.rela.plt) } :rodata

  /* All exception handling sections */
  .gcc_except_table : { *(.gcc_except_table .gcc_except_table.*) } :rodata
  .eh_frame_hdr : {
    __eh_frame_hdr_start__ = .;
    *(.eh_frame_hdr)
    __eh_frame_hdr_end__ = .;
  } :rodata
  .eh_frame : { KEEP (*(.eh_frame)) } :rodata

  /* Misc .rodata stuffs (build-id, ect) */
  .note.gnu.build-id : { *(.note.gnu.build-id) } :rodata

  /* Read-write sections */
  . = ALIGN(0x1000);

  /* App data */
  .data : {
    *(.data .data.*)
  } :data

  /* This section should be made read only after relocation but in practice we will not do that */
  .data.rela.ro : {
    *(.data.rela.ro.local*)
    *(.data.rela.ro .data.rela.ro.*)
  } :data

  /* This section should be made read only after relocation but in practice we will not do that */
  .data.rel.ro : {
    *(.data.rel.ro.local*)
    *(.data.rel.ro .data.rel.ro.*)
  } :data

  /* All GOT sections */
  .got : { *(.got.plt) *(.igot.plt) *(.got) *(.igot) } :data

  /* The dynamic section as we need it to be stored in the binary */
  .dynamic : {
    HIDDEN(__dynamic_start__ = .);
    *(.dynamic)
  } :data :dynamic

  /* Align for .init_array/.fini_array */
  . = ALIGN(8);

  /* App init array */
  .init_array : {
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP (*(.init_array))
  } :data

  /* App fini array */
  .fini_array : {
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
    KEEP (*(.fini_array))
  } :data

  /* Thread Local sections */

  .tdata : {
    __tdata_align_abs__ = ABSOLUTE(.);
    __tdata_start__ = .;
    *(.tdata .tdata.*)
    __tdata_end__ = .;
  } :data

  .tbss : {
    __tbss_align_abs__ = ABSOLUTE(.);
    __tbss_start__ = .;
    *(.tbss .tbss.*)
    *(.tcommon)
    __tbss_end__ = .;
  } :data

  /* BSS section */
  . = ALIGN(0x1000);

  .bss : {
    HIDDEN(__bss_start__ = .);
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(8);
    HIDDEN(__bss_end__ = .);
    . = ALIGN(0x1000);
  } :data

  __end__ = ABSOLUTE(.);

  HIDDEN(__argdata__ = .);

  /DISCARD/ : {
    /* No need of the interpreter */
    *(.interp)
  }
}