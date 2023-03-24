from ctypes import *
dll = cdll.LoadLibrary("./test.so")
dll.f0_void_void()

#dll.f1_int_int.argtypes = [c_int]
#dll.f1_int_int.restype = c_int
out_int = dll.f1_int_int(67)
print(str(out_int))

#dll.f2_pchar_pchar.argtypes = [c_char_p]
dll.f2_pchar_pchar.restype = c_char_p
out_txt = dll.f2_pchar_pchar(b"test text")
print(str(out_txt))

class NODE(Structure):
    pass

NODE._fields_ = [
    ("name", c_char_p),
    ("val", c_int),
    ("next", POINTER(NODE))
]

#dll.f3_node_node.argtypes = [POINTER(NODE), c_char_p]
dll.f3_node_node.restype = POINTER(NODE)

node0 = NODE(b"node0", 57, None)
node1 = NODE(b"node1", 58, pointer(node0))
node2 = NODE(b"node2", 59, pointer(node1))
node3 = NODE(b"node3", 60, pointer(node2))
node = dll.f3_node_node(pointer(node3), b"node0")
print(str(node[0].name))

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

PY_CALLBACK = CFUNCTYPE(None, c_int, c_char_p)
#dll.f6_call_callback.argtypes = [PY_CALLBACK, c_int, c_char_p]
def py_callback(i, s):
    print(str(i) + " " + str(s))

dll.f6_call_callback(PY_CALLBACK(py_callback), 33, b"Text from python")

i = c_int.in_dll(dll, "g_val_int")
print("Integer exported from 'so': " + str(i))

class API(Structure):
    pass

API._fields_ = [
    ("f0", CFUNCTYPE(None)),
    ("f1", CFUNCTYPE(c_int, c_int)),
    ("f2", CFUNCTYPE(c_char_p, c_char_p))
]

print('Structure based API from "C" ...')
api = API.in_dll(dll, "g_api")
api.f0()
api.f1(77)
api.f2(b"API test")


print('Exported int array from "C" ...')
arr_int = (c_int * 4).in_dll(dll, "g_arr_int")
print("First and last values: " + str(arr_int[0]), "... " + str(arr_int[2]))

print('Function pointer array based API from "C" ...')
PY_FUNC = CFUNCTYPE(None)
arr_func = (PY_FUNC * 3).in_dll(dll, "g_arr_func")
arr_func[0]()
arr_func[1]()

node = pointer(node3)
arr_func[2](pointer(node), b"node0")
print('Found node: ' + str(node[0].name))
