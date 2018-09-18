using namespace std;
#include <iostream>
#include <fstream>         // std::file
#include <string>         // std::string
#include <cstddef>       // std::size_t
#include <algorithm>    // std::sort
#include <tuple>
#include <vector>

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
    node left;
    node right;
}node;

typedef vector< tuple<unsigned int,unsigned char, bool> > tuple_vector;


int treatAsText(ifstream &inputFile);

node* makeHeap(tuple_vector &sortedTupleVector);
tuple_vector buildFrequencies(ifstream &inputFile, int bytes);
tuple_vector sortTupleVector(tuple_vector &tupleVector);
int findInTupleVector(tuple_vector tupleVector, unsigned char character);
void incrementCharInTupleVector(tuple_vector &tupleVector, int indexOfChar);
void appendNewCharInTupleVector(tuple_vector &tupleVector, unsigned char character);
void insertInSortedTupleVector(tuple_vector &sortedTupleVector, unsigned int value);
void eraseInSortedTupleVector(tuple_vector &sortedTupleVector, unsigned int value);
void linkNodesToHeadNode(node* head, node* left, node* right);
void printTupleVector(tuple_vector &tupleVector);

int main(int argc, char* argv[]) {
    ifstream inputFile;
    string fileName;

    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
     {
        return errno;
     }
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
    cout << "The current working directory is "<< cCurrentPath << endl;
        
    if(argc < 2) {
        cout << "Type path to file: ";
        cin >> fileName;
    }
    else
        fileName = argv[1];

    inputFile.open (fileName);

    if(!inputFile) {
        cerr << "Invalid path. File not found." << endl;
        return -2;
    }

    size_t extensionMark = fileName.find_last_of('.');
    string extension = fileName.substr(extensionMark+1, fileName.length());

    if(extension == "txt")
    {
        treatAsText(inputFile);
    }
    
    inputFile.close();
    return 0;
}

int treatAsText(ifstream &inputFile){
    tuple_vector frequencyTupleVector = buildFrequencies(inputFile, 1);
    frequencyTupleVector = sortTupleVector(frequencyTupleVector);

    makeHeap(frequencyTupleVector);
    return 0;
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
    vector<node> nodeList;
    node head, left, right;

    // while(static_cast<unsigned int>(sortedTupleVector.size()) > 0){
        if(sortedTupleVector.size() != 1){
            int lowestFrequency = get<0>(sortedTupleVector[0]);
            int secondLowestFrequency = get<0>(sortedTupleVector[1]);

            int newValueForNode = lowestFrequency + secondLowestFrequency;

            head = {.dataFrequency = newValueForNode, .internal = true, .left = left, .right = right};

            eraseInSortedTupleVector(sortedTupleVector, lowestFrequency);
            insertInSortedTupleVector(sortedTupleVector, newValueForNode);

        // }
    
    }


    return nullptr;
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

void linkNodesToHeadNode(node* head, node* left, node* right){

}
