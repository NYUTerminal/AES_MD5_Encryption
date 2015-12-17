#include <iostream>
#include<fstream>

#include<vector>
#include <iterator>

#include<cstring>
#include <string>
#include<sstream>
#include <iomanip>

#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/err.h>

using namespace std;

int filewrite(vector<char> data, char* filename)
{
    std::vector<char>::const_iterator i;
    ofstream myfile (filename, ios::out | ios::binary);
    //vector<char> data;
    //std::copy(data1.begin(), data1.end(), std::back_inserter(data));
    std::copy(data.begin(), data.end(), std::ostream_iterator<char>(myfile, ""));

    return ((data.end()-data.begin())*8);
}

vector<char> readfile(char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    vector<char> read_key =       std::vector<char>((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    //std::copy(read_key.begin(), read_key.end(), std::ostream_iterator<char>(cout, ""));
    return read_key;
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    cout<<"Some Error"<<endl;
    abort();
}

vector<char> MAC_comp(char* text,int text_len)
{
    EVP_MD_CTX *mdctx;
    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, text, text_len);
    char md_value[32];
    int md_len;
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_destroy(mdctx);
    vector<char> message_digest;
    message_digest.assign(md_value,md_value+32 );
//    std::copy(message_digest.begin(), message_digest.end(), std::ostream_iterator<char>(cout, ""));
    return message_digest;
}

int encrypt( char* plaintext, int plaintext_len,  char* key,char* iv,  char* ciphertext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;
    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

vector<char> generate_fk(char *key, char *filename)
{
    char* iv="01234567890123456";
    char ciphertext[32];
    int cipher_len = encrypt( filename, strlen(filename),  key,iv,  ciphertext);
    vector<char> hash_value = MAC_comp(ciphertext,cipher_len);
    return hash_value;

}

std::string get_readable_hash(vector<char> myhash)
{
    char fin[60];
    int i,j;
    j=0;
    for(i=0; i<myhash.size(); i++)
    {
        stringstream ss;
        ss<<std::setw(8)<< std::setfill ('x')<<std::hex <<(int) myhash[i];
        string temp = ss.str();

        fin[j]=temp[6];
        j++;
        fin[j]=temp[7];
        j++;
    }
    return string(fin,64);
}


int main(int argc, char* argv[])
{


       if (argc < 2)          {
           std::cout << "Usage is preprocess <shared_keyfile> <file_to_encrypt>";            std::cin.get();
           exit(0);
       }
       char* keyfile_path = argv[1];
       char* fn1_path = argv[2];

    
    vector<char>temp_hash = MAC_comp(fn1_path,strlen(fn1_path));
    string readable_temp_hash = get_readable_hash(temp_hash);
    string fn1_hash = string(keyfile_path,(strlen(keyfile_path)-6));
    if (readable_temp_hash.compare(fn1_hash) != 0)
    {
        cout<<"The filename you are trying to upload is not associated with the key. Please verify the file-key pair.";
        getchar();
        exit(0);
    }

    vector<char> key_vec = readfile(keyfile_path);
    vector<char> plaintext_vec = readfile(fn1_path);
    vector<char> f1_hash_vec = MAC_comp(&plaintext_vec[0], plaintext_vec.size());
    vector<char> payload_vec;
    payload_vec.insert(payload_vec.end(),plaintext_vec.begin(),plaintext_vec.end());
    payload_vec.insert(payload_vec.end(),f1_hash_vec.begin(),f1_hash_vec.end());
    //std::copy(payload_vec.begin(), payload_vec.end(), std::ostream_iterator<char>(cout, ""));
    char* iv="01234567890123456";

    char ciphertext[payload_vec.size()+16];
    int cipher_len= encrypt(&payload_vec[0],payload_vec.size(),&key_vec[0],iv,ciphertext);

    char* cipher_end = ciphertext+(cipher_len);
    vector<char> cipher_vec(ciphertext, cipher_end);

    //To make a name for the file
    string fn2_final = string(keyfile_path,(strlen(keyfile_path)-6));
    //fn2_final.append(".data");

    int success = filewrite(cipher_vec,&fn2_final[0]);
    cout<<success<<" Encrypted file generated"<<endl;
    
//To upload to file to dropbox
    std::string filename = "upload.py ";
    std::string command = "python ";
    command += filename;
    command += fn2_final;
    system(command.c_str());
    cout<<"Following file has a Local copy and also uploaded to DropBox : "<<fn2_final<<endl;
    getchar();
    //Write fk to a file by name fn2.share.key.

}

