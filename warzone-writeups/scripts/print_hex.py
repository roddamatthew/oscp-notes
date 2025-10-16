"""
Reverse the endianness of 32-bit words and print as characters
"""

s = "75620041 74315F37 7334775F 625F376E 33745572 7230665F 62343363 00216531"
words = s.split()

new_string = ""
for word in words:
    new_string = new_string + bytes.fromhex(word)[::-1].decode()
print(new_string)