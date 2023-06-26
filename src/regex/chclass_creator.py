import string

if __name__ == "__main__":
    l = map(ord, string.ascii_letters + string.digits + "_")
    ccls = 0
    for bit in l:
        ccls |= 1 << bit
    
    chunks = [hex((ccls >> i) & ((1<<64) - 1)) + "ULL" for i in range(0, 256, 64)]
    print(f"(uint64_t[4]){{{', '.join(chunks)}}}")