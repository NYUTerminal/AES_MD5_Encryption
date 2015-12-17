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

vector<char> fileread(char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);
    // read the data:
    vector<char> read_key =  std::vector<char>((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
    //std::copy(read_key.begin(), read_key.end(), std::ostream_iterator<char>(cout, ""));
    return read_key;
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    cout<<"Decryption Failed"<<endl;
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
        // cout<<myhash[i] << " : " <<temp[6]<<temp[7]<<endl;

    }
    return string(fin,64);
}

int decrypt(char *ciphertext, int ciphertext_len,  char *key,
            char *iv,  char *plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}




int main(int argc, char* argv[])
{

    if (argc < 2)   
    {
        std::cout << "Usage is recover <shared_keyfile> <file_to_decrypt>"; // Inform the user of how to use the program
        std::cin.get();
        exit(0);
    }
    //TODO Find the proper path.
    cout<<"Downloading File from DropBox";
    std::string filename = "download.py ";
    std::string command = "python ";
    command += filename;
    command += argv[2];
    system(command.c_str());

    char* keyfile_path = argv[1];
    char* fn2_path = argv[2];


    vector<char> key_vec = fileread(keyfile_path);
    vector<char> cipher_vec = fileread(fn2_path);
     char* iv="01234567890123456";

    char payload[cipher_vec.size()+16];
    int payload_len= decrypt(&cipher_vec[0],cipher_vec.size(),&key_vec[0],iv,payload);

    vector<char> f1(payload,(payload+(payload_len-32)));
    vector<char> mac_code((payload+(payload_len-32)), (payload+payload_len));

    vector<char> myhash = MAC_comp(&f1[0], f1.size());
    string passed_hash = get_readable_hash(mac_code);
    string calculated_hash = get_readable_hash(myhash);

    if(passed_hash.compare(calculated_hash)!=0){
        cout<< "The file was altered,in the transmission. Hence discarding the file.";
        getchar();
        exit(0);
    }
    cout<<"MAC codes matched. Integrity test passed. Now copying the decrypted text."<<endl;
    string fn3 = string(fn2_path,strlen(fn2_path));
    fn3.append(".data");
    filewrite(f1,&fn3[0]);
    cout<<"File Written"<<fn3<<" as the file. You may now use this file.";
    getchar();




}

