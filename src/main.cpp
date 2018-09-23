using namespace std;
#include <iostream>
#include <fstream>   // std::file
#include <string>    // std::string
#include <cstddef>   // std::size_t
#include <algorithm> // std::sort
#include <tuple>
#include <vector>
#include <stack>
#include <unordered_map>
#include <bitset>
#include <queue>

#define DEBUG 0

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

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

#pragma region[Function headers]
int encodeText(ifstream &inputFile, ofstream &outputFile);

node *makeHeap(tuple_vector &sortedTupleVector);

tuple_vector buildFrequencies(ifstream &inputFile, int bytes);
tuple_vector sortTupleVector(tuple_vector &tupleVector);
int findInTupleVector(tuple_vector tupleVector, unsigned char character);
void incrementCharInTupleVector(tuple_vector &tupleVector, int indexOfChar);
void appendNewCharInTupleVector(tuple_vector &tupleVector, unsigned char character);

void makeMap(node *rootNode, string str, unordered_map<unsigned char, string> &map);

void printTupleVector(tuple_vector &tupleVector);
void printCodes(node *root, string str);

void writeEncodedFile(node *root, ifstream &inputFile, ofstream &outputFile, unordered_map<unsigned char, string> map);
void writeCodeTable(ofstream &newFile, node *root, string str);
node* recognizeCodeTable(ifstream &codeFile);
void decodeEncodedFile(ifstream &inputFile, ofstream &outputFile, node *root);
#pragma endregion

int main(int argc, char *argv[])
{
    ifstream inputFile;
    ofstream outputFile;
    string inputFileName;
    string outputFileName;

    if (argc < 2)
    {
        cout << "Type path to file: ";
        cin >> inputFileName;
    }
    else
        inputFileName = (string) argv[1];

    if (argc > 2)
        outputFileName = (string) argv[2];
    else    
    {
        cout << "Type output file name: ";
        cin >> outputFileName;
    }
    
    outputFile.open(outputFileName);
    inputFile.open(inputFileName);

    if (!inputFile)
    {
        cerr << "Invalid path. File not found." << endl;
        return -2;
    }

    unsigned int extensionMark = static_cast<unsigned int>(inputFileName.find_last_of('.'));
    string extension = inputFileName.substr(extensionMark + 1, inputFileName.length());

    if (extension == "txt")
    {
        encodeText(inputFile,outputFile);
    }
    else
    {
        cerr << "File is not .txt, exiting" << endl;
    }

    inputFile.close();
    return 0;
}

#pragma region [Functions implementation]
int encodeText(ifstream &inputFile, ofstream &outputFile){
    tuple_vector frequencyTupleVector = buildFrequencies(inputFile, 1);
    frequencyTupleVector = sortTupleVector(frequencyTupleVector);

    node *rootNode = makeHeap(frequencyTupleVector);
    unordered_map<unsigned char, string> mapOfCode;
    makeMap(rootNode, "", mapOfCode);

    ofstream encodedFile;
    encodedFile.open("encodedFile.dat", ios_base::binary);
    writeEncodedFile(rootNode, inputFile, encodedFile, mapOfCode);
    encodedFile.close();

    ifstream readCodeFile;
    readCodeFile.open("codeFile.dat", fstream::binary);
    node* newRoot = recognizeCodeTable(readCodeFile);
    readCodeFile.close();

    ifstream encodedFileDecode;
    encodedFileDecode.open("encodedFile.dat", ios_base::binary);
    decodeEncodedFile(encodedFileDecode, outputFile, newRoot);
    encodedFileDecode.close();
    return 0;
}

node *makeHeap(tuple_vector &sortedTupleVector)
{
    stack<node *> nodeStack;
    stack<node *> nodeDestack;
    node *head, *left, *right, *dequeue;

    unsigned int tupleVectorSize = static_cast<unsigned int>(sortedTupleVector.size());

    for (int i = tupleVectorSize - 1; i >= 0; i--)
    {
        nodeStack.push(new node(get<1>(sortedTupleVector[i]), get<0>(sortedTupleVector[i]), false));
    }

    while (nodeStack.size() > 1)
    {
        left = nodeStack.top();
        nodeStack.pop();

        right = nodeStack.top();
        nodeStack.pop();

        unsigned int newFrequencyValue = left->dataFrequency + right->dataFrequency;

        head = new node('0', newFrequencyValue, true);
        head->left = left;
        head->right = right;

        unsigned int stackSize = nodeStack.size();
        for (unsigned int i = 0; i < stackSize; i++)
        {
            dequeue = nodeStack.top();
            if (dequeue->dataFrequency < newFrequencyValue)
            {
                nodeDestack.push(dequeue);
                nodeStack.pop();
            }
            else
            {
                nodeStack.push(head);
                break;
            }
        }
        if (nodeStack.size() <= 0 && nodeDestack.size() > 0)
            nodeStack.push(head);

        while (nodeDestack.size() > 0)
        {
            dequeue = nodeDestack.top();
            nodeStack.push(dequeue);
            nodeDestack.pop();
        }
    }

    return head;
}


tuple_vector buildFrequencies(ifstream &inputFile, int bytes)
{
    char readByte;
    tuple_vector frequencyTupleVector;

    while (inputFile)
    {
        inputFile.read(&readByte, bytes);
        unsigned char readCharacter = (unsigned char)readByte;
        int index = findInTupleVector(frequencyTupleVector, readCharacter);

        if (index < 0)
        {
            appendNewCharInTupleVector(frequencyTupleVector, readCharacter);
        }
        else
        {
            incrementCharInTupleVector(frequencyTupleVector, index);
        }
    }

    return frequencyTupleVector;
}

tuple_vector sortTupleVector(tuple_vector &tupleVector)
{
    unsigned int sizeOfList = static_cast<unsigned int>(tupleVector.size());
    vector<int> arrayOfFrequencies;

    for (unsigned int i = 0; i < sizeOfList; i++)
    {
        arrayOfFrequencies.push_back(get<0>(tupleVector[i]));
    }
    sort(arrayOfFrequencies.begin(), arrayOfFrequencies.end());

    tuple_vector sortedTupleVector;
    unsigned int currentFrequency;
    unsigned char character;

    //sort lazely
    for (unsigned int i = 0; i < sizeOfList; i++)
    {
        currentFrequency = arrayOfFrequencies[i];
        for (unsigned int y = 0; y < sizeOfList; y++)
        {
            if (currentFrequency == get<0>(tupleVector[y]))
            {
                character = get<1>(tupleVector[y]);
                sortedTupleVector.push_back(make_tuple(currentFrequency, character, false));

                tupleVector[y].operator=(make_tuple(0, character, false));
            }
        }
    }

    return sortedTupleVector;
}

int findInTupleVector(tuple_vector tupleVector, unsigned char character)
{
    unsigned int sizeOfList = static_cast<unsigned int>(tupleVector.size());
    for (unsigned int i = 0; i < sizeOfList; i++)
    {
        if (get<1>(tupleVector[i]) == character)
            return i;
    }
    return -1;
}

void appendNewCharInTupleVector(tuple_vector &tupleVector, unsigned char character)
{
    tupleVector.push_back(make_tuple(1, character, false));
}
void incrementCharInTupleVector(tuple_vector &tupleVector, int indexOfChar)
{
    int currentFrequency = get<0>(tupleVector[indexOfChar]);
    unsigned char character = get<1>(tupleVector[indexOfChar]);
    tupleVector[indexOfChar].operator=(make_tuple(++currentFrequency, character, false));
}

void makeMap(node *rootNode, string str, unordered_map<unsigned char, string> &map)
{
    if (!rootNode)
        return;

    if (!rootNode->internal)
        map.insert({rootNode->dataByte, str});

    makeMap(rootNode->left, str + "0", map);
    makeMap(rootNode->right, str + "1", map);
}

void printTupleVector(tuple_vector &tupleVector)
{
    unsigned int currentFrequency;
    unsigned char character;
    bool isInternalNode;
    unsigned int sizeOfList = static_cast<unsigned int>(tupleVector.size());

    cout << "frequency    character   internalNode" << endl;
    for (unsigned int i = 0; i < sizeOfList; i++)
    {
        currentFrequency = get<0>(tupleVector[i]);
        character = get<1>(tupleVector[i]);
        isInternalNode = get<2>(tupleVector[i]);
        cout << currentFrequency;
        cout << "\t";
        cout << "\t";
        cout << character;
        cout << "\t";
        cout << "\t";
        cout << isInternalNode << endl;
    }
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

void writeEncodedFile(node *root, ifstream &inputFile, ofstream &outputFile, unordered_map<unsigned char, string> map)
{
    printCodes(root, "");

    ofstream codeFile;
    codeFile.open("codeFile.dat", ios_base::binary);
    writeCodeTable(codeFile, root, "");
    codeFile.close();

    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    char readByte;

    bitset<32> buffer;
    bitset<32> bufferOfBuffer;
    unsigned int bufferIterator = 0;
    bool activateSecondBuffer = false;
    bool reachedBufferBits = false;
    bool bufferEmpty = true;

    while (inputFile)
    {
        inputFile.read(&readByte, 1);
        unsigned char readCharacter = (unsigned char)readByte;
        string readString = map[readCharacter];
        unsigned int sizeOfReadString = readString.size();
        unsigned int amountOfBitsToWrite;
        unsigned int leftoverBitsToWrite;

        if (bufferIterator + sizeOfReadString > 32)
        {
            activateSecondBuffer = true;
            reachedBufferBits = true;
            amountOfBitsToWrite = 32 - bufferIterator;
            leftoverBitsToWrite = sizeOfReadString - amountOfBitsToWrite;
        }
        else
        {
            amountOfBitsToWrite = sizeOfReadString;
        }

        unsigned int sharedIndex;
        for (sharedIndex = 0; sharedIndex < amountOfBitsToWrite; sharedIndex++)
        {
            if (readString[sharedIndex] == '1')
                buffer[bufferIterator] = true;
            else
                buffer[bufferIterator] = false;
            bufferIterator++;
        }

        if (activateSecondBuffer)
        {
            for (unsigned int i = 0; i < leftoverBitsToWrite; i++)
            {
                if (readString[sharedIndex++] == '1')
                    bufferOfBuffer[i] = true;
                else
                    bufferOfBuffer[i] = false;
            }
        }

        if (reachedBufferBits)
        {
            outputFile.write((char *)&buffer, sizeof(buffer));
            // #ifdef DEBUG
            //     string newStr = buffer.to_string();
            //     reverse(newStr.begin(),newStr.end());
            //     cout << newStr;
            // #endif
            bufferEmpty = true;
            reachedBufferBits = false;
            bufferIterator = 0;
            if (activateSecondBuffer)
            {
                buffer = bufferOfBuffer;
                bufferIterator = leftoverBitsToWrite;
                bufferEmpty = false;
                activateSecondBuffer = false;
            }
        }
    }
    if (!bufferEmpty)
    {
        for (unsigned int i = bufferIterator; i < 32 - bufferIterator; i++)
        {
            buffer[i] = false;
        }
        outputFile.write((char *)&buffer, sizeof(buffer));
    }
}

void writeCodeTable(ofstream &newFile, node *root, string str)
{
    if (!root)
        return;

    if (!root->internal)
    {
        bitset<8> dataToBits = root->dataByte;
        newFile << dataToBits;
        bitset<5> sizeBits = str.length();
        newFile << sizeBits;

        bitset<16> stringToBits;
        for (unsigned int i = 0; i < str.length(); i++)
        {
            if(str[i] == '1')
                stringToBits[i]=true;
            else
                stringToBits[i]=false;
        }
        newFile << stringToBits;

    }

    writeCodeTable(newFile, root->left, str + "0");
    writeCodeTable(newFile, root->right, str + "1");
}

node* recognizeCodeTable(ifstream &codeFile){
    node *root = new node(0,0,true);
    node *nodeIterator;

    bitset<8> dataToBits;
    bitset<5> sizeBits;
    bitset<16> stringToBits;

    char buffer[29];
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
        internalIndex = 15;
        for (int i = 13; i < 29; i++)
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

void decodeEncodedFile(ifstream &inputFile, ofstream &outputFile, node *root)
{
    
    char readByte;
    node *nodeIterator;

    int leftOff = 0;

    queue<bool> boolQueue;
    bool reducedToValue = true;
    while (inputFile)
    {
        if (boolQueue.empty())
        {
            leftOff = 0;
            inputFile.read(&readByte, 1);
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

            if (currentBool == false)
                nodeIterator = nodeIterator->left;
            else
                nodeIterator = nodeIterator->right;

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
#pragma endregion
