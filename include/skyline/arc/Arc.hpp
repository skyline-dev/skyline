namespace skyline {
namespace arc {
    class Arc {
        public:

        struct PACKED Hash40 {
            u32 Hash;
            u8 Length;

            operator u64() const{
                return (((u64)Length) << 32 | Hash) & 0xFFFFFFFFFFLL;
            }
        };
    };
};
};