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

typedef vector< tuple<unsigned int,unsigned char> > tuple_vector;

struct node{
    tuple<unsigned int,unsigned char> dataTuple;
    node* left;
    node* right;
}node;

int treatAsText(ifstream &inputFile);

tuple_vector buildFrequencies(ifstream &inputFile, int bytes);
tuple_vector sortTupleVector(tuple_vector &tupleList);
int findInTupleVector(tuple_vector tupleList, unsigned char character);
void incrementCharInTupleVector(tuple_vector &tupleList, int indexOfChar);
void appendNewCharInTupleVector(tuple_vector &tupleList, unsigned char character);
void printTupleVector(tuple_vector &tupleList);

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

int findInTupleVector(tuple_vector tupleList, unsigned char character){
    unsigned int sizeOfList = static_cast<unsigned int>(tupleList.size());
    for(unsigned int i = 0; i < sizeOfList; i++){
        if(get<1>(tupleList[i]) == character)
            return i;
    }
    return -1;
}

void appendNewCharInTupleVector(tuple_vector &tupleList, unsigned char character){
    tupleList.push_back(make_tuple(1, character));
}
void incrementCharInTupleVector(tuple_vector &tupleList, int indexOfChar){
    int currentFrequency = get<0>(tupleList[indexOfChar]);
    unsigned char character = get<1>(tupleList[indexOfChar]);
    tupleList[indexOfChar].operator=(make_tuple(++currentFrequency, character));
}

void printTupleVector(tuple_vector &tupleList){
    unsigned int currentFrequency;
    unsigned char character;
    unsigned int sizeOfList = static_cast<unsigned int>(tupleList.size());

    cout<<"frequency    character"<<endl;
    for(unsigned int i = 0; i < sizeOfList; i++){  
        currentFrequency = get<0>(tupleList[i]);
        character = get<1>(tupleList[i]);
        cout << currentFrequency;
        cout << "\t";
        cout << "\t";
        cout << character << endl;
    }
}

tuple_vector sortTupleVector(tuple_vector &tupleList){
    unsigned int sizeOfList = static_cast<unsigned int>(tupleList.size());
    vector<int> arrayOfFrequencies;

    for(unsigned int i = 0; i < sizeOfList; i++){ 
        arrayOfFrequencies.push_back(get<0>(tupleList[i]));
    }
    sort(arrayOfFrequencies.begin(), arrayOfFrequencies.end());

    tuple_vector sortedVector;
    unsigned int currentFrequency;
    unsigned char character;

    //sort lazely
    for(unsigned int i = 0; i < sizeOfList; i++){ 
        currentFrequency = arrayOfFrequencies[i];
        for(unsigned int y = 0; y < sizeOfList; y++){ 
            if(currentFrequency == get<0>(tupleList[y])) {
                character = get<1>(tupleList[y]);
                sortedVector.push_back(make_tuple(currentFrequency, character));
            
                tupleList[y].operator = (make_tuple(0, character));
            }   
        }
    }

    #if DEBUG
        printTupleVector(sortedVector);
    #endif

    return sortedVector;
}