# Include the Dropbox SDK
import dropbox
from hashlib import md5
from Crypto.Cipher import AES
from Crypto import Random
import hashlib
from os.path import basename
import os
import sys
# Get your app key and secret from the Dropbox developer website
app_key = 'nudrph66fzkbry1'
app_secret = '73clw5o3drgaqmu'

flow = dropbox.client.DropboxOAuth2FlowNoRedirect(app_key, app_secret)

def download(u_file):
        f, metadata = client.get_file_and_metadata(u_file)
        out = open(u_file, 'wb')
        out.write(f.read())
        out.close()
        # print metadata

# password = "adityacryptoproject2"
# in_file = "sample.txt"
# out_file = "sample_en.txt"


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
u_file = sys.argv[1]
print u_file
download(u_file)
