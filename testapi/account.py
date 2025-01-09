import base64
import random
from string import ascii_letters, digits  # so we don't have to type [A-Za-z0-9] by hand

# this code works only on python 3.6 and above

possible_letters = ascii_letters + digits


def generate_rs(n: int) -> str:
    return ("").join(random.choices(possible_letters, k=n))

def encode_gjp(password: str) -> str:
    # put it through the xor cipher with the key "37526")
    encoded = xor_cipher(password, "37526")
    # encode the password to base64
    encoded_base64 = base64.b64encode(encoded.encode()).decode()
    encoded_base64 = encoded_base64.replace("+", "-")
    encoded_base64 = encoded_base64.replace("/", "_")
    return encoded_base64

def decode_gjp(gjp: str) -> str:
    # decode the password from base64
    decoded_base64 = base64.b64decode(gjp.encode()).decode()
    # put it through the xor cipher with the key "37526")
    decoded = xor_cipher(decoded_base64, "37526")

    return decoded

def generate_uuid(parts: [int] = (8, 4, 4, 4, 10)) -> str:
    # apply generate_rs to each number in parts, then join results
    return ("-").join(map(generate_rs, parts))

def generate_udid(start: int = 100_000, end: int = 100_000_000) -> str:
    return "S" + str(random.randint(start, end))

print(generate_udid())