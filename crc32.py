"""
Taken from https://www.w3.org/TR/png/#D-CRCAppendix

W3C CRC-32 algorithm used in gzip and png specs.

I think I need to find the relevant crc params and use an existing python package instead.
"""

# Table of CRCs of all 8-bit messages.
crc_table = {}

# Flag: has the table been computed? Initially false.
crc_table_computed = False


# Make the table for a fast CRC. */
def make_crc_table():
    global crc_table_computed
    for i in range(256):
        c = i
        for j in range(8):
            if (c & 1):
                c = 0xedb88320 ^ (c >> 1)
            else:
                c = c >> 1

        crc_table[i] = c

    crc_table_computed = True


def update_crc(crc: int, buf: bytes, len: int):
    """
    Update a running CRC with the bytes buf[0..len-1]--the CRC
    should be initialized to all 1's, and the transmitted value
    is the 1's complement of the final running CRC (see the
    crc() routine below).
    """
    c = crc
    if not crc_table_computed:
        make_crc_table()
    for i in range(len):
        c = crc_table[(c ^ buf[i]) & 0xff] ^ (c >> 8)

    return c


# Return the CRC of the bytes buf[0..len-1]. */
def crc32(buf: bytes, len: int):
    return update_crc(0xffffffff, buf, len) ^ 0xffffffff
