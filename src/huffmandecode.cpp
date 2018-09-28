using namespace std;
#include <iostream>
#include <fstream>   // std::file
#include <string>    // std::string
#include <tuple>
#include <vector>
#include <stack>
#include <unordered_map>
#include <bitset>
#include <queue>

#pragma warning (disable : 4068 ) /* disable unknown pragma warnings */

#pragma region[Type definitions]
typedef struct node
{
    unsigned int dataFrequency;
    unsigned char dataByte;
    bool internal;
    node *left;
    node *right;

    node(unsigned char dataByte, unsigned int dataFrequency, bool isInternal)
    {
        left = NULL;
        right = NULL;
        this->internal = isInternal;
        this->dataFrequency = dataFrequency;
        this->dataByte = dataByte;
    }

} node;
typedef vector<tuple<unsigned int, unsigned char, bool>> tuple_vector;
#pragma endregion

int decodeExample(int argc, char *argv[]);
node* recognizeCodeTable(ifstream &codeFile);
void decodeEncodedFile(ifstream &codeTableFile, ofstream &outputFile, node *root);
void printCodes(node *root, string str);

int decodeExample(int argc, char *argv[])
{
    string codeTableFileName;
    string encodedFileName;
    string outputFileName;

    if (argc < 2)
    {
        cout << "Type path to code table dat file: ";
        cin >> codeTableFileName;
    }
    else
        codeTableFileName = (string) argv[1];

    if (argc < 3)
    {
        cout << "Type path to encoded text dat file: ";
        cin >> encodedFileName;
    }
    else
        encodedFileName = (string) argv[2];

    if (argc < 4){
        cout << "Type output file name: ";
        cin >> outputFileName;
    }
    else    
    {
        outputFileName = (string) argv[3];
    }

    
    ifstream codeTableFile;
    codeTableFile.open(codeTableFileName);

    ofstream outputFile;
    outputFile.open(outputFileName);

    if (!codeTableFile)
    {
        cerr << "Invalid path. File not found." << endl;
        return -2;
    }

    unsigned int extensionMark = static_cast<unsigned int>(codeTableFileName.find_last_of('.'));
    string extension = codeTableFileName.substr(extensionMark + 1, codeTableFileName.length());

    if (extension != "dat")
    {
        cerr << "File is not .dat, exiting" << endl;
        return -2;
    }

    ifstream readCodeFile;
    readCodeFile.open(codeTableFileName, fstream::binary);
    node* newRoot = recognizeCodeTable(readCodeFile);
    readCodeFile.close();
    codeTableFile.close();

    printCodes(newRoot, "");

    ifstream encodedFileDecode;
    encodedFileDecode.open(encodedFileName, ios_base::binary);
    decodeEncodedFile(encodedFileDecode, outputFile, newRoot);
    encodedFileDecode.close();
    return 0;
}

void printCodes(node *root, string str)
{
    if (!root)
        return;

    if (!root->internal)
        cout << root->dataByte << ": " << str << "\n";

    printCodes(root->left, str + "0");
    printCodes(root->right, str + "1");
}

node* recognizeCodeTable(ifstream &codeFile){
    node *root = new node(0,0,true);
    node *nodeIterator;

    bitset<8> dataToBits;
    bitset<5> sizeBits;
    bitset<32> stringToBits;

    char buffer[45];
    int stringSize = 0;

    while (codeFile.read((char *)&buffer,sizeof(buffer)))
    {
        nodeIterator = root;
        for (int i = 0; i < 8; i++)
        {
            if(buffer[i] == '1')
                dataToBits[7-i]=true;
            else
                dataToBits[7-i]=false;
        }
        int internalIndex = 4;
        for (int i = 8; i < 13; i++)
        {
            if(buffer[i] == '1')
                sizeBits[internalIndex--]=true;
            else
                sizeBits[internalIndex--]=false;
        }
        internalIndex = 31;
        for (int i = 13; i < 45; i++)
        {
            if(buffer[i] == '1')
                stringToBits[internalIndex--]=true;
            else
                stringToBits[internalIndex--]=false;
        }

        stringSize = static_cast<unsigned int>(sizeBits.to_ulong());

        for(int i = 0; i < stringSize; i++){
            if(stringToBits[i] == false){
                if(!nodeIterator->left){
                    if(i >= stringSize-1){
                        unsigned char newChar = static_cast<unsigned char>(dataToBits.to_ulong());
                        nodeIterator->left = new node(newChar,0, false);
                    }
                    else{
                        nodeIterator->left = new node(0,0, true);
                        nodeIterator = nodeIterator->left;
                    }
                }
                else{
                    nodeIterator = nodeIterator->left;
                }
            }
            else{
                if(!nodeIterator->right){
                    if(i >= stringSize-1){
                        unsigned char newChar = static_cast<unsigned char>(dataToBits.to_ulong());
                        nodeIterator->right = new node(newChar,0, false);
                    }
                    else{
                        nodeIterator->right = new node(0,0,true);
                        nodeIterator = nodeIterator->right;
                    }
                }
                else{
                    nodeIterator = nodeIterator->right;
                }
            }
        }
    }

    return root;
}

void decodeEncodedFile(ifstream &codeTableFile, ofstream &outputFile, node *root)
{
    
    char readByte;
    node *nodeIterator;

    int leftOff = 0;

    queue<bool> boolQueue;
    bool reducedToValue = true;
    while (codeTableFile)
    {
        if (boolQueue.empty())
        {
            leftOff = 0;
            codeTableFile.read(&readByte, 1);
            for (int i = 0; i < 8; i++)
            {
                boolQueue.push((readByte >> i) & 1);
            }
        }

        if (reducedToValue)
        {
            nodeIterator = root;
            reducedToValue = false;
        }
        for (int i = leftOff; i < 8; i++)
        {
            bool currentBool = boolQueue.front();
            boolQueue.pop();

            if (currentBool == false){
                if(nodeIterator->left == NULL)
                    {
                        cerr << "Invalid codeTable" << endl;
                        return;
                    }
                nodeIterator = nodeIterator->left;
            }
            else{
                if(nodeIterator->right == NULL)
                {
                    cerr << "Invalid codeTable" << endl;
                    return;
                }
                nodeIterator = nodeIterator->right;
            }

            if (!nodeIterator->internal)
            {
                outputFile << nodeIterator->dataByte;
                reducedToValue = true;
                leftOff = i;
                break;
            }
            if (boolQueue.empty())
                break;
        }
    }
}