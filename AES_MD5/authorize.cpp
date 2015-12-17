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
    ofstream myfile (filename, ios::out | ios::binary);    //std::copy(data1.begin(), data1.end(), std::back_inserter(data));
    std::copy(data.begin(), data.end(), std::ostream_iterator<char>(myfile, ""));

    return ((data.end()-data.begin())*8);
}

vector<char> fileread(char* filename)
{
    
    std::ifstream file(filename, std::ios::binary);

    vector<char> read_key =  std::vector<char>((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
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

    if (argc < 2)       {
        std::cout << "Usage is authorize <keyfile> <file_to_encrypt>";         	  std::cin.get();
        exit(0);
    }
    char* keyfile_path = argv[1];
    char* fn1_path = argv[2];
        vector<char> key = fileread(argv[0]);
    vector<char> fk = generate_fk(&key[0],fn1_path);
    vector<char> fn2_vector = MAC_comp(fn1_path, strlen(fn1_path));
    string fn2_name = get_readable_hash(fn2_vector);
    fn2_name.append(".share");

    int success = filewrite(fk,&fn2_name[0]);
    cout<<success<<" shared_key generated"<<endl;
    cout<<" Share file Key:"<<fn2_name<<endl<<"The key to be shared to end user"<<endl;
    getchar();
   

}

