#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

/*
w - wordsize in bits. It is 32 bits
r - number of rounds
b - number of bytes in the user key --- 16 bytes
*/

char hexvalue(int i)
{
    return (i >= 0 && i <= 9) ? '0' + i : 'A' + i - 10;
}
std::string unint_to_hex(unsigned int A)
{
    stringstream stream;
    stream << hex << setw(8) << setfill('0') << A;
    return stream.str();
}
std::string little_endian(std::string str)
{
    std::string res;
    if (str.length() % 2 != 0)
    {
        str = '0' + str;
    }

    for (int i = str.length() - 1; i >= 0; i = i - 2)
    {
        res += str[i - 1];
        res += str[i];
    }

    return res;
}

std::string hexdecimal_to_string(unsigned int A, unsigned int B, unsigned int C, unsigned int D)
{

    std::string res;
    res += little_endian(unint_to_hex(A)) + little_endian(unint_to_hex(B)) + little_endian(unint_to_hex(C)) + little_endian(unint_to_hex(D));
    return res;
}

int left_rotation(unsigned int a, unsigned int b, unsigned int w)
{
    unsigned int log_w = (unsigned int)log2(w); // circular shift 
    b <<= w - log_w;
    b >>= w - log_w;
    return (a << b) | (a >> (w - b));
}

int right_rotation(unsigned int a, unsigned int b, unsigned int w)
{
    unsigned int log_w = (unsigned int)log2(w); // circular shift 
    b <<= w - log_w;
    b >>= w - log_w;
    return (a >> b) | (a << (w - b));
}

std::string encryption(unsigned int w, unsigned int r, unsigned int b, unsigned int log_w, int64_t mod, const std::string &text, unsigned int *S)
{

    std::string result;
    unsigned int A, B, C, D;
    A = strtoul(little_endian(text.substr(0, 8)).c_str(), NULL, 16);
    B = strtoul(little_endian(text.substr(8, 8)).c_str(), NULL, 16);
    C = strtoul(little_endian(text.substr(16, 8)).c_str(), NULL, 16);
    D = strtoul(little_endian(text.substr(24, 8)).c_str(), NULL, 16);
    int32_t t, u, temp;

    B += S[0];
    D += S[1];
    for (int i = 1; i <= r; ++i)
    {
        t = left_rotation((B * (2 * B + 1)) % mod, log_w, w);
        u = left_rotation((D * (2 * D + 1)) % mod, log_w, w);
        A = left_rotation((A ^ t), u, w) + S[2 * i];
        C = left_rotation((C ^ u), t, w) + S[2 * i + 1];
        temp = A;
        A = B;
        B = C;
        C = D;
        D = temp;
    }

    A += S[2 * r + 2];
    C += S[2 * r + 3];

    result = hexdecimal_to_string(A, B, C, D);
    return result;
}

std::string decryption(unsigned int w, unsigned int r, unsigned int b, unsigned int log_w, int64_t mod, const std::string &text, unsigned int *S)
{

    std::string result;

    unsigned int A, B, C, D;
    A = strtoul(little_endian(text.substr(0, 8)).c_str(), NULL, 16);
    B = strtoul(little_endian(text.substr(8, 8)).c_str(), NULL, 16);
    C = strtoul(little_endian(text.substr(16, 8)).c_str(), NULL, 16);
    D = strtoul(little_endian(text.substr(24, 8)).c_str(), NULL, 16);

    unsigned int t, u, temp;

    C -= S[2 * r + 3];
    A -= S[2 * r + 2];
    for (int i = r; i >= 1; --i)
    {
        temp = D;
        D = C;
        C = B;
        B = A;
        A = temp;
        u = left_rotation((D * (2 * D + 1)) % mod, log_w, w);
        t = left_rotation((B * (2 * B + 1)) % mod, log_w, w);
        C = right_rotation((C - S[2 * i + 1]) % mod, t, w) ^ u;
        A = right_rotation((A - S[2 * i]) % mod, u, w) ^ t;
    }
    D -= S[1];
    B -= S[0];

    result = hexdecimal_to_string(A, B, C, D);

    return result;
}

unsigned int *key_schedule(std::string key, int w, int r, int b, int64_t mod)
{
    unsigned int w_bytes = w / 8; // 1 byte = 8bits
    unsigned int c = b / w_bytes;

    unsigned int *L = (unsigned int *)malloc(c * sizeof(int));

    for (int i = 0; i < c; i++)
    {
        L[i] = strtoul(little_endian(key.substr(w_bytes * 2 * i, w_bytes * 2)).c_str(), NULL, 16);
    }

    unsigned int *S = (unsigned int *)malloc((2 * r + 4) * sizeof(int));
    unsigned int p = 0xB7E15163, q = 0x9E3779B9; // need to change

    unsigned int log_w = (unsigned int)log2(w);
    // int64_t mod = std::pow(2, w);

    S[0] = p;
    for (int i = 1; i <= (2 * r + 3); i++)
    {
        S[i] = (S[i - 1] + q) % mod;
    }

    unsigned int A = 0, B = 0, i = 0, j = 0;
    int v = 3 * std::max(c, (unsigned int)(2 * r + 4));

    for (int s = 1; s <= v; s++)
    {
        A = S[i] = left_rotation((S[i] + A + B) % mod, 3, w);
        B = L[j] = left_rotation((L[j] + A + B) % mod, (A + B), w);
        i = (i + 1) % (2 * r + 4);
        j = (j + 1) % c;
    }

    return S;
}


int generate(int argc, char *argv[], std::string &key, std::string &text, std::string &mode)
{
    if (argc != 3)
    {
        std::cout << "invalid format \n";
        std::cout << "FORMAT :\n";
        std::cout << "./<file_name>  <input_filename> <output_filename> \n";
        return -1;
    }

    std::fstream input;
    input.open(argv[1], std::ios::in | std::ios::out);
    std::string line;
    int count = 0;

    if (input.is_open())
    {
        while (!input.eof())
        {
            getline(input, line);
            switch (count)
            {

            case 0:
                if (line.compare(0, strlen("Encryption"), "Encryption") == 0 ||
                    line.compare(0, strlen("Decryption"), "Decryption") == 0)
                {
                    mode = line;
                }
                else
                {
                    std::cout << "Invalid Usage \n";
                    exit(0);
                }
                break;

            case 1:

                if ((mode == "Encryption" && line.compare(0, strlen("Plaintext :"), "Plaintext :") == 0))
                {
                    text = line.substr(strlen("Plaintext :"));
                }
                else if ((mode == "Decryption" && line.compare(0, strlen("Ciphertext :"), "Ciphertext :") == 0))
                {
                    text = line.substr(strlen("Ciphertext :"));
                }
                else
                {
                    std::cout << "Invalid Usage \n";
                    std::cout << " 'Text' should be used";
                    exit(0);
                }
                break;

            case 2:

                if (line.compare(0, strlen("Key :"), "Key :") == 0)
                {
                    key = line.substr(strlen("Key :"));
                }
                else
                {
                    std::cout << "Invalid Usage \n";
                    std::cout << " 'Key :' should be used";
                    exit(0);
                }
                break;

            default:

                key += line;
                break;
            }

            count++;
        }
    }
    else
    {
        std::cout << "Unable to open file \n";
        return -1;
    }

    return 0;
}

void remove_space(std::string &str)
{
    string result;
    for (char c : str) {
        if (!isspace(c)) {
            result += c;
        }
    }
    str = result;
}


void file_style(int argc, char *argv[]){
    unsigned int w, r, b, log_w;
    int64_t mod;
    std::string key, text, mode;

    if (generate(argc, argv, key, text, mode) != 0)
    {
        return;
    }

    remove_space(key);
    remove_space(text);

    if (text.length() % 32 != 0)
    {
        while (text.length() % 32 != 0)
        {
            text = text + "0";
        }
    }

    w = 32;               // one word=32bit
    r = 20;               // no.of rounds
    b = key.length() / 2; // ex: 02 53 is of 2 bytes
    mod = std::pow(2, w);
    log_w = (unsigned int)log2(w);

    unsigned int *S = key_schedule(key, w, r, b, mod);

    if (mode == "Encryption")
    {
        //std::cout << "hi";
        std::string ciphertext = encryption(w, r, b, log_w, mod, text, S);
        //std::cout << "hi";
        std::string result;
        for (int i = 0; i < ciphertext.length(); i = i + 2)
        {
            result += ciphertext[i];
            result += ciphertext[i + 1];
            result += ' ';
        }
        //std::cout << "hi";
        std::fstream output;
        output.open(argv[2], std::ios::trunc | std::ios::out);

        if (output.is_open())
        {
            //std::cout << "hi1";
            output << "Ciphertext :" << result << std::endl;
        }
        else
        {
            std::cout << "Unable to open output file";
        }

        output.close();
    }
    else if (mode == "Decryption")
    {

        std::string plaintext = decryption(w, r, b, log_w, mod, text, S);
        std::string result;
        for (int i = 0; i < plaintext.length(); i = i + 2)
        {
            result += plaintext[i];
            result += plaintext[i + 1];
            result += ' ';
        }

        std::fstream output;
        output.open(argv[2], std::ios::trunc | std::ios::out);

        if (output.is_open())
        {
            output << "Plaintext :" << result << std::endl;
            output.close();
        }
        else
        {
            std::cout << "Unable to open output file   " << strerror(errno);
        }
    }
    
    std::cout << "suseecful, check the files. ";
    return;
}

void terminal_style(){

    cout << "Enter 1 for Encryption" << '\n';
    cout << "Enter 2 for Decryption" << '\n';
    int select;

    while(1){
      
      cin >> select;
      cin.ignore();
      if(select!=1 && select!=2){
        cout << "select valid mode(either 1 or 2)";
      }
      else{
        break;
      }
    }

    

        unsigned int w,r,b,lw;
        int64_t pw;    
        string k,text;

        cout << "Enter Plaintext to encrypt : ";
        getline(cin,text,'\n');

      while(1){
        cout << "Enter key : ";
        getline(cin,k,'\n');
        if(k.length()<8){
            cout<< "this key is not valid. minimum 8 lenght for key "<<'\n'<<"key format is hexadecimal.."<<endl;
        }
        else break;
      }

        remove_space(k);
        remove_space(text);

        if(text.length() % 32 != 0){
            while(text.length() % 32 != 0){
                text=text+"0";
            }
        }

        w = 32;
        r = 20;
        b = k.length()/2;
        pw = pow(2, w); 
        lw = (unsigned int)log2(w);
        unsigned int * S = key_schedule(k,w,r,b,pw);   

    if(select == 1){
        
        cout << "Encrypted text is : ";
        for(int j=0;j<text.length();j=j+32){
            string ciphertext = encryption(w,r,b,lw,pw,text.substr(j,32),S);
            for(int i = 0;i<ciphertext.length();i= i+2){
                cout << ciphertext[i] << ciphertext[i+1] << " ";
            }
        }
    }
    else if(select == 2){
        
        cout << "Decrypted text is : ";
        for(int j=0;j<text.length();j=j+32){
            string plaintext = decryption(w,r,b,lw,pw,text.substr(j,32),S);
            for(int i = 0;i<plaintext.length();i= i+2){
                cout << plaintext[i] << plaintext[i+1] << " ";
            }
        }

    }
    return;
}

int main(int argc, char *argv[])
{
   
    if(argc == 1){
        terminal_style(); 
    }
    else{
       file_style(argc,argv);
    }
    
    return 0;
   
}