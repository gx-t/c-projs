from ctypes import *
dll = cdll.LoadLibrary("./test.so")
dll.f0_void_void()
class NODE(Structure):
    pass

NODE._fields_ = [
    ("name", c_char_p),
    ("val", c_int),
    ("next", POINTER(NODE))
]

dll.f3_node_node.argtypes = [POINTER(NODE), c_char_p]
dll.f3_node_node.restype = POINTER(NODE)

node0 = NODE(b"node0", 57, None)
node1 = NODE(b"node1", 58, pointer(node0))
node2 = NODE(b"node2", 59, pointer(node1))
node3 = NODE(b"node3", 60, pointer(node2))
node = dll.f3_node_node(pointer(node3), b"node0")
print(str(node))

dll.f4_load_pem_file_print_subject_based_string.argtypes = [c_char_p, c_char_p]
dll.f4_load_pem_file_print_subject_based_string.restype = None
dll.f4_load_pem_file_print_subject_based_string(b'ed25519-self-signed-1.pem', b'test-text-test-text')

pem_str = ''
with open('ed25519-self-signed-1.pem') as ff:
    pem_str = ff.read()

dll.f5_load_pem_string_check_2018_cert.argtypes = [c_char_p, c_uint32]
dll.f5_load_pem_string_check_2018_cert.restype = c_int
res = dll.f5_load_pem_string_check_2018_cert(pem_str.encode(), len(pem_str))
print('ed25519-self-signed-1.pem: ' + str(res))

with open('ed25519-self-signed-2.pem') as ff:
    pem_str = ff.read()

res = dll.f5_load_pem_string_check_2018_cert(pem_str.encode(), len(pem_str))
print('ed25519-self-signed-2.pem: ' + str(res))


