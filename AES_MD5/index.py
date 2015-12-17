# Include the Dropbox SDK
import dropbox
from hashlib import md5
from Crypto.Cipher import AES
from Crypto import Random
import hashlib
from os.path import basename
import os
# Get your app key and secret from the Dropbox developer website
app_key = 'nudrph66fzkbry1'
app_secret = '73clw5o3drgaqmu'

flow = dropbox.client.DropboxOAuth2FlowNoRedirect(app_key, app_secret)

   
def derive_key_and_iv(password, salt, key_length, iv_length):
    d = d_i = ''
    while len(d) < key_length + iv_length:
        d_i = md5(d_i + password + salt).digest()
        d += d_i
    return d[:key_length], d[key_length:key_length+iv_length]

def encrypt(in_file, out_file, password, key_length=32):
    bs = AES.block_size
    salt = Random.new().read(bs - len('Salted__'))
    key, iv = derive_key_and_iv(password, salt, key_length, bs)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    out_file.write('Salted__' + salt)
    finished = False
    while not finished:
        chunk = in_file.read(1024 * bs)
        if len(chunk) == 0 or len(chunk) % bs != 0:
            padding_length = (bs - len(chunk) % bs) or bs
            chunk += padding_length * chr(padding_length)
            finished = True
        out_file.write(cipher.encrypt(chunk))

def decrypt(in_file, out_file, password, key_length=32):
    bs = AES.block_size
    salt = in_file.read(bs)[len('Salted__'):]
    key, iv = derive_key_and_iv(password, salt, key_length, bs)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    next_chunk = ''
    finished = False
    while not finished:
        chunk, next_chunk = next_chunk, cipher.decrypt(in_file.read(1024 * bs))
        if len(next_chunk) == 0:
            padding_length = ord(chunk[-1])
            chunk = chunk[:-padding_length]
            finished = True
        out_file.write(chunk)

def md5(in_fname,out_fname):
    hash = hashlib.md5()
    with open(in_fname, "rb") as in_md5 ,open(out_fname, "wb") as out_md5 :
        for chunk in iter(lambda: in_md5.read(4096), b""):
            hash.update(chunk)
    out_md5.write(hash.hexdigest())

def exists(path):
    try:
        dbx.files_get_metadata(path)
        return True
    except:
        return False

def upload(u_file):
        f = open(u_file, 'rb')
        response = client.put_file(u_file, f)
        print "updated and uploaded encrypted integrity file:", response
        f.close()



password = "adityacryptoproject2"
in_file = "sample.txt"
out_file = "sample_en.txt"


# Have the user sign in and authorize this token
authorize_url = flow.start()
print '1. Go to: ' + authorize_url
print '2. Click "Allow" (you might have to log in first)'
print '3. Copy the authorization code.'
code = raw_input("Enter the authorization code here: ").strip()

# This will fail if the user enters an invalid authorization code
access_token, user_id = flow.finish(code)

client = dropbox.client.DropboxClient(access_token)



print 'linked account: ', client.account_info()

print "choose any option:"
print "Enter 1 for uploading a file"
print "Enter 2 for Downloading a file"
option = input()
print option
if(option == 1):
    u_file = ("Enter the file name to  be uploaded:")
    integrity_path = "integrity_en.txt"
    file_exists= exists(integrity_path)
    if(file_exists == True):
        upload(u_file)
    else:
        #create integrity file and then call upload
        f = ("integrity_en.txt", 'rb')
        response = client.put_file('/integriten.txt',f)
        f.close()
        upload(u_file)

else:
    print "wasdesd"
    pass



# with open(in_filename, 'rb') as in_file, open(out_filename, 'wb') as out_file:
#     decrypt(in_file, out_file, password)

# f = open('sample.txt', 'rb')
# response = client.put_file('/magnum-opus.txt', f)
# print 'uploaded: ', response

# folder_metadata = client.metadata('/')
# print 'metadata: ', folder_metadata

# f, metadata = client.get_file_and_metadata('/magnum-opus.txt')
# out = open('magnum-opus.txt', 'wb')
# out.write(f.read())
# out.close()
# print metadata