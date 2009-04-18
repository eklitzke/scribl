Scribl On-Disk Log Format
=========================

The format of the scribl logs is very simple:
    [4 bytes seconds] [4 bytes useconds] [records]

The format of a record is:
    [uint8_t len] [identifier, len bytes] [8 bytes, "double" counter val]

Note: identifiers actually cannot contain null byts, so the len prefix is
actually not needed. But it is provided as an optimization for reading the file.
