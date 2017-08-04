
S_OK         = 0
S_FALSE      = 0x1
E_PENDING    = 0x8000000a
E_UNEXPECTED = 0x8000ffff
E_FAIL       = 0x80004005

d = {
    0            : 'S_OK',
    0x1          : 'S_FALSE',
    0x8000000a   : 'E_PENDING',
    0x8000ffff   : 'E_UNEXPECTED',
    0x80004005   : 'E_FAIL'
}

def get(*args):
    global d
    return d.get(*args)
