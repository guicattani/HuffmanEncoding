using namespace std;
#include <iostream>
#include <fstream>         // std::file
#include <string>         // std::string
#include <cstddef>       // std::size_t
#include <algorithm>    // std::sort
#include <tuple>
#include <vector>
#include <stack>          
#include <unordered_map>   
#include <bitset>   
#include <queue>   

#define DEBUG 1

#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

typedef struct node{
    unsigned int dataFrequency;
    unsigned char dataByte;
    bool internal;
    node *left;
    node *right;

    node(unsigned char dataByte, unsigned int dataFrequency, bool isInternal){
        left = NULL;
        right = NULL;
        this->internal = isInternal;
        this->dataFrequency = dataFrequency;
        this->dataByte = dataByte;
    }

}node;

typedef vector< tuple<unsigned int,unsigned char, bool> > tuple_vector;


int encodeText(ifstream &inputFile);

node* makeHeap(tuple_vector &sortedTupleVector);
tuple_vector buildFrequencies(ifstream &inputFile, int bytes);
tuple_vector sortTupleVector(tuple_vector &tupleVector);
int findInTupleVector(tuple_vector tupleVector, unsigned char character);
void incrementCharInTupleVector(tuple_vector &tupleVector, int indexOfChar);
void appendNewCharInTupleVector(tuple_vector &tupleVector, unsigned char character);
void printTupleVector(tuple_vector &tupleVector);
void printCodes(node* root, string str);
void makeMap(node* rootNode, string str, unordered_map<unsigned char, string> &map );

void writeEncodedFile(node* root, ifstream &inputFile, ofstream &outputFile, unordered_map<unsigned char, string> map);
void writeCodeTable(ofstream &newFile, node* root, string str);

void decodeEncodedFile(ifstream &inputFile, node* root );

int main(int argc, char* argv[]) {
    ifstream inputFile;
    string fileName;

    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
     {
        return errno;
     }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    cout << "The current working directory is "<< cCurrentPath << endl;
    
    bool encodeOperation;
    if(argc < 2) {
        cout << "Type path to file: ";
        cin >> fileName;
    }
    else if(argc < 3){
        fileName = argv[2];
        if(argc < 4)
            encodeOperation = false;
    }

    inputFile.open (fileName);

    if(!inputFile) {
        cerr << "Invalid path. File not found." << endl;
        return -2;
    }

    unsigned int extensionMark = static_cast<unsigned int> (fileName.find_last_of('.'));
    string extension = fileName.substr(extensionMark+1, fileName.length());

    if(extension == "txt")
    {
        encodeText(inputFile);
    }
    else{
        cerr << "File is not .txt, exiting" << endl;
    }
    
    inputFile.close();
    return 0;
}

int encodeText(ifstream &inputFile){
    tuple_vector frequencyTupleVector = buildFrequencies(inputFile, 1);
    frequencyTupleVector = sortTupleVector(frequencyTupleVector);

    node* rootNode = makeHeap(frequencyTupleVector);
    unordered_map<unsigned char, string> mapOfCode;
    makeMap(rootNode, "", mapOfCode);

    ofstream outputFile;
    outputFile.open("output.dat",ios_base::binary);
    writeEncodedFile(rootNode, inputFile, outputFile, mapOfCode);
    outputFile.close();

    ifstream encodedFile;
    encodedFile.open("output.dat",ios_base::binary);
    decodeEncodedFile(encodedFile, rootNode);
    encodedFile.close();
    return 0;
}

void makeMap(node* rootNode, string str, unordered_map<unsigned char, string> &map){
    if (!rootNode)
        return;
 
    if (!rootNode->internal)
        map.insert({rootNode->dataByte, str});
 
    makeMap(rootNode->left, str+ "0", map);
    makeMap(rootNode->right, str + "1",  map);
}

tuple_vector buildFrequencies(ifstream &inputFile, int bytes){
    char readByte;
    tuple_vector frequencyTupleVector;

    while(inputFile){
        inputFile.read(&readByte, bytes);
        unsigned char readCharacter = (unsigned char) readByte;
        int index = findInTupleVector(frequencyTupleVector, readCharacter);
        
        if(index < 0){
            appendNewCharInTupleVector(frequencyTupleVector, readCharacter);
        }
        else{
            incrementCharInTupleVector(frequencyTupleVector,index);
        }
    }

    #if DEBUG
        printTupleVector(frequencyTupleVector);
    #endif

    return frequencyTupleVector;
}

int findInTupleVector(tuple_vector tupleVector, unsigned char character){
    unsigned int sizeOfList = static_cast<unsigned int>(tupleVector.size());
    for(unsigned int i = 0; i < sizeOfList; i++){
        if(get<1>(tupleVector[i]) == character)
            return i;
    }
    return -1;
}

void appendNewCharInTupleVector(tuple_vector &tupleVector, unsigned char character){
    tupleVector.push_back(make_tuple(1, character, false));
}
void incrementCharInTupleVector(tuple_vector &tupleVector, int indexOfChar){
    int currentFrequency = get<0>(tupleVector[indexOfChar]);
    unsigned char character = get<1>(tupleVector[indexOfChar]);
    tupleVector[indexOfChar].operator=(make_tuple(++currentFrequency, character, false));
}

void printTupleVector(tuple_vector &tupleVector){
    unsigned int currentFrequency;
    unsigned char character;
    bool isInternalNode;
    unsigned int sizeOfList = static_cast<unsigned int>(tupleVector.size());

    cout<<"frequency    character   internalNode"<<endl;
    for(unsigned int i = 0; i < sizeOfList; i++){  
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

tuple_vector sortTupleVector(tuple_vector &tupleVector){
    unsigned int sizeOfList = static_cast<unsigned int>(tupleVector.size());
    vector<int> arrayOfFrequencies;

    for(unsigned int i = 0; i < sizeOfList; i++){ 
        arrayOfFrequencies.push_back(get<0>(tupleVector[i]));
    }
    sort(arrayOfFrequencies.begin(), arrayOfFrequencies.end());

    tuple_vector sortedTupleVector;
    unsigned int currentFrequency;
    unsigned char character;

    //sort lazely
    for(unsigned int i = 0; i < sizeOfList; i++){ 
        currentFrequency = arrayOfFrequencies[i];
        for(unsigned int y = 0; y < sizeOfList; y++){ 
            if(currentFrequency == get<0>(tupleVector[y])) {
                character = get<1>(tupleVector[y]);
                sortedTupleVector.push_back(make_tuple(currentFrequency, character, false));
            
                tupleVector[y].operator = (make_tuple(0, character, false));
            }   
        }
    }

    #if DEBUG
        printTupleVector(sortedTupleVector);
    #endif

    return sortedTupleVector;
}

node* makeHeap(tuple_vector &sortedTupleVector){
    stack<node*> nodeStack;
    stack<node*> nodeDestack;
    node *head, *left, *right, *dequeue;

    unsigned int tupleVectorSize = static_cast<unsigned int>(sortedTupleVector.size());

    for(int i = tupleVectorSize-1; i >= 0; i--){
        nodeStack.push(new node(get<1>(sortedTupleVector[i]),get<0>(sortedTupleVector[i]),false) );
    }
     
     while(nodeStack.size() > 1){
        left = nodeStack.top();
        nodeStack.pop();

        right = nodeStack.top();
        nodeStack.pop();

        unsigned int newFrequencyValue = left->dataFrequency + right->dataFrequency;

        head = new node('0',newFrequencyValue, true);
        head->left = left;
        head->right = right;

        unsigned int stackSize = nodeStack.size();
        for(unsigned int i = 0; i < stackSize; i++){
            dequeue = nodeStack.top();
            if(dequeue->dataFrequency < newFrequencyValue){
                nodeDestack.push(dequeue);
                nodeStack.pop();
            }
            else{
                nodeStack.push(head);
                break;
            }
        }
        if(nodeStack.size() <= 0 && nodeDestack.size() > 0)
            nodeStack.push(head);

        while(nodeDestack.size() > 0) {
            dequeue = nodeDestack.top();
            nodeStack.push(dequeue);
            nodeDestack.pop();
        }
    
    }
    
    #if DEBUG
        printCodes(head,"");
    #endif

    return head;
}

void writeEncodedFile(node* root, ifstream &inputFile, ofstream &outputFile, unordered_map<unsigned char, string> map){
    //writeCodeTable(outputFile, root, "");
    //outputFile << "\n";

    inputFile.clear();
    inputFile.seekg(0, ios::beg);
    char readByte;

    bitset<32> buffer;
    bitset<32> bufferOfBuffer;
    unsigned int bufferIterator = 0;
    bool activateSecondBuffer = false;
    bool reachedBufferBits = false;
    bool bufferEmpty = true;

    while(inputFile){
        inputFile.read(&readByte, 1);
        unsigned char readCharacter = (unsigned char) readByte;
        string readString = map[readCharacter];
        unsigned int sizeOfReadString = readString.size();
        unsigned int amountOfBitsToWrite;
        unsigned int leftoverBitsToWrite;

        if(bufferIterator + sizeOfReadString > 32){
            activateSecondBuffer = true;
            reachedBufferBits = true;
            amountOfBitsToWrite = 32 - bufferIterator;
            leftoverBitsToWrite = sizeOfReadString - amountOfBitsToWrite;
        }
        else{
            amountOfBitsToWrite = sizeOfReadString;
        }

        for(unsigned int i = 0; i < amountOfBitsToWrite; i++){
            if(readString[i] == '1')
                buffer[bufferIterator] = true;
            else    
                buffer[bufferIterator] = false;
            bufferIterator++;
        }

        if(activateSecondBuffer){
            for(unsigned int i = 0; i < leftoverBitsToWrite; i++){
                if(readString[i] == '1')
                    bufferOfBuffer[i] = true;
                else    
                    bufferOfBuffer[i] = false;
            }   
        }

        #if DEBUG
            // cout << "read" << readString << endl;
        #endif

        if(reachedBufferBits){
            outputFile.write( (char*)&buffer, sizeof(buffer) );
            bufferEmpty = true;
            reachedBufferBits = false;
            bufferIterator = 0;
            if(activateSecondBuffer){
                buffer = bufferOfBuffer;
                bufferIterator = leftoverBitsToWrite;
                bufferEmpty = false;
            }
        }
        
    }
    if(!bufferEmpty){
        for(unsigned int i = bufferIterator; i < 32 - bufferIterator; i++){
                buffer[i] = false;
        }   
        outputFile.write( (char*)&buffer, sizeof(buffer) );
    }
}
void writeCodeTable(ofstream &newFile, node* root, string str){
    if (!root)
        return;
 
    if (!root->internal){
        newFile << root->dataByte <<" "<< str <<" ";
        #if DEBUG
            cout<< root->dataByte <<" "<< str <<" ";
        #endif
    }
 
    writeCodeTable(newFile,root->left, str + "0");
    writeCodeTable(newFile, root->right, str + "1");
}

void printCodes(node* root, string str)
{
    if (!root)
        return;
 
    if (!root->internal)
        cout << root->dataByte << ": " << str << "\n";
 
    printCodes(root->left, str + "0");
    printCodes(root->right, str + "1");
}

void insertInSortedTupleVector(tuple_vector &sortedTupleVector, unsigned int value){
    unsigned int sizeOfList = static_cast<unsigned int>(sortedTupleVector.size());
    tuple_vector::iterator iterator = sortedTupleVector.begin();

    if(get<0>(sortedTupleVector[0]) > value){
        sortedTupleVector.insert(iterator,make_tuple(value,0,true));
        return;
    }
    
    for(unsigned int i = 1; i < sizeOfList; i++){
        if(get<0>(sortedTupleVector[i]) > value){
            sortedTupleVector.insert(iterator+i,make_tuple(value,0,true));
            return;
        }
    }
    sortedTupleVector.insert(sortedTupleVector.end(),make_tuple(value,0,true));
    return;
}

void eraseInSortedTupleVector(tuple_vector &sortedTupleVector, unsigned int value){
    unsigned int sizeOfList = static_cast<unsigned int>(sortedTupleVector.size());
    tuple_vector::iterator iterator = sortedTupleVector.begin();
    
    for(unsigned int i = 0; i < sizeOfList; i++){
        if(get<0>(sortedTupleVector[i]) == value){
            if(sizeOfList != 1){
                sortedTupleVector.erase(iterator+i);
                sortedTupleVector.erase(iterator+i);
                return;
            }
            else {
                sortedTupleVector.erase(iterator+i);
                return;
            }
        }
    }

}

void decodeEncodedFile(ifstream &inputFile, node* root ){
    char readByte;
    node* nodeIterator;

    int leftOff = 0;

    queue<bool> boolQueue;
    bool reducedToValue = true;
    while(inputFile){
        if(boolQueue.empty()){
            leftOff = 0;
            inputFile.read(&readByte, 1);
            for(int i = 0; i < 8; i++){
                boolQueue.push((readByte >> i) & 1);
            }
        }
    
        if(reducedToValue){
            nodeIterator = root;
            reducedToValue = false;
        }
        for(int i = leftOff; i < 8; i++){
            bool currentBool = boolQueue.front();
            boolQueue.pop();
            
            if(currentBool == false)
                nodeIterator = nodeIterator->left;
            else
                nodeIterator = nodeIterator->right;

            if (!nodeIterator->internal){
                cout << nodeIterator->dataByte;
                reducedToValue = true;
                leftOff = i;
                break;
            }
            if(boolQueue.empty())
                break;
        }   
    }
}