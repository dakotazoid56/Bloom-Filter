#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <chrono>

using namespace std;


const int N = 10000000;          //Size of the Universe
const int n = 10000;              //Number of Items in Subset S
const float c = 10;             //Constant for Size of subset
const int m = int(c * n);       //Size of Hash Table
const int k = 5;                //Number of HashFunctions

int hashSeeds[k];               //Seeds for Hash Functions 1

int prime;                      //Smallest mersennePrime greater than N
int hashA[k];                   //Seeds for A Hash Functions 2
int hashB[k];                   //Seeds for B Hash Functions 2


void GenerateSeeds(){
    random_device programRandomNum;
    mt19937 randomNum(programRandomNum());
    uniform_int_distribution<int> seedOptions(1, INT32_MAX);
    for (int& seed : hashSeeds){
        seed = seedOptions(randomNum);
    }
}

void GeneratePrime(){
    std::vector<int> mersennePrimes = {2, 3, 5, 7, 13, 17, 19, 31, 61, 
        89, 107, 127, 521, 607, 1279, 2203, 2281, 3217, 4253, 4423, 9689, 9941, 11213, 
        19937, 21701, 23209, 44497, 86243, 110503, 132049, 216091, 756839, 859433, 1257787};
    
    for (int p : mersennePrimes){
        long long curPrime = (1LL << p) - 1;
        if(curPrime > INT32_MAX){
            cout << "PRIME IS BIGGER THAN INT: " << curPrime << endl;
            return;
        }
        if (curPrime > N){
            prime = curPrime;
            return;
        }
    }
    cout << "RAN OUT OF BIG PRIME NUMBERS!!!" << endl;
}

void GenerateAB(){
    random_device programRandomNum;

    mt19937 randomNumA(programRandomNum());
    mt19937 randomNumB(programRandomNum());

    uniform_int_distribution<int> primeDisA(1, prime-1);
    uniform_int_distribution<int> primeDisB(0, prime-1);

    for (int& a : hashA){
        a = primeDisA(randomNumA);
    }
    for (int& b : hashB){
        b = primeDisB(randomNumB);
    }
}


int FirstHashFunction(int i, int x, int mTemp){
    mt19937 curSeed(hashSeeds[i] + x);
    uniform_int_distribution<int> tableSize(0, mTemp-1);
    
    //Make Random Number fit inside HashTable
    int curLoc = tableSize(curSeed);
    return curLoc;
}
int SecondHashFunction(int i, int x, int mTemp) {
    unsigned long long prod = static_cast<unsigned long long>(hashA[i]) * static_cast<unsigned long long>(x);
    unsigned long long curVal = (prod + hashB[i]) % prime;
    return static_cast<int>(curVal % mTemp);
}


class BloomFilter {
private:

    int m;                                  //size of hash Table
    int k;                                  //number of hash functions
    bool firstHash;                         //Deciding which hash function to use
    std::vector<pair<bool, int>> T;         //Hash Table T

public:
    BloomFilter(int mTemp, int kTemp, bool f = true) : m(mTemp), k(kTemp), firstHash(f), T(mTemp, make_pair(false, 0)) {
    }

    ~BloomFilter(){
    }
    
    void add(int x){
        for(int i = 0; i < this->k; i++){
            int curLoc;

            if(firstHash){
                curLoc = FirstHashFunction(i, x, this->m);
            }
            else{
                curLoc = SecondHashFunction(i, x, this->m);
            }
            T[curLoc].first = true;
            T[curLoc].second += 1;
        }
    }
    bool contains(int x){
        for(int i = 0; i < this->k; i++){
            int curLoc;

            if(firstHash){
                curLoc = FirstHashFunction(i, x, this->m);
            }
            else{
                curLoc = SecondHashFunction(i, x, this->m);
            }

            if(!T[curLoc].first){
                return false;
            }
        }
        return true;
    }
    map<int, float> getBinCounts() const{
        map<int, float> countMap;

        for(const auto& bin : T) {
            if (bin.first){
                countMap[bin.second]++;
            }
            else{
                countMap[0] ++;
            }
        }
        return countMap;
    }
};

void PopulateCsvFromMap(string fileName, map<int, float> map){
    ofstream outfile(fileName);
    for (const auto& i : map) {
        outfile << i.first << "," << i.second << "\n";  // Example: x=i, y=i^2
    }
    outfile.close();
}

unordered_set<int> PopulateBloomFilters(BloomFilter& b1, BloomFilter& b2, int size, int largestNum){
    random_device programRandomNum;
    mt19937 gen(programRandomNum());
    uniform_int_distribution<int> subsetDis(1, largestNum-1);

    unordered_set<int> subset;
    int num;
    while (subset.size() < static_cast<size_t>(size)){
        num = subsetDis(gen);
        if (subset.insert(num).second) {
            b1.add(num);
            b2.add(num);
        }
    }

    return subset;
}
unordered_set<int> PopulateBloomFilter(BloomFilter& b1, int size, int largestNum){
    random_device programRandomNum;
    mt19937 gen(programRandomNum());
    uniform_int_distribution<int> subsetDis(1, largestNum-1);

    unordered_set<int> subset;
    int num;
    while (subset.size() < static_cast<size_t>(size)){
        num = subsetDis(gen);
        if (subset.insert(num).second) {
            b1.add(num);
        }
    }

    return subset;
}

void HashTestRandomInput(int size, int tempN, int tempn, int tempm, int tempk,  int totalRuns) {
    map<int, float> b1TotalMap;
    map<int, float> b2TotalMap;
    for(int i = 0; i < totalRuns; i++){
        BloomFilter b1(tempm, tempk, true);
        BloomFilter b2(tempm, tempk, false);
        GenerateSeeds();
        GeneratePrime();
        GenerateAB();

        unordered_set<int> subset = PopulateBloomFilters(b1, b2, size, tempN);

        map<int, float> b1TempMap = b1.getBinCounts();
        map<int, float> b2TempMap = b2.getBinCounts();
        for (const auto& bin : b1TempMap){
            b1TotalMap[bin.first] += bin.second;
        }
        
        for (const auto& bin : b2TempMap){
            b2TotalMap[bin.first] += bin.second;
        }
    }
    for (auto& key : b1TotalMap){
        b1TotalMap[key.first] = b1TotalMap[key.first] / totalRuns;
    }
    for (auto& key : b2TotalMap){
        b2TotalMap[key.first] = b2TotalMap[key.first] / totalRuns;
    }
    PopulateCsvFromMap("firstHash.csv", b1TotalMap);
    PopulateCsvFromMap("secondHash.csv", b2TotalMap);
}

void HashTestLinearInput(int size, int tempm, int tempk, int totalRuns, int numInterval) {
    map<int, float> b1TotalMap;
    map<int, float> b2TotalMap;
    

    for(int i = 0; i < totalRuns; i++){
        int num = 0;

        GenerateSeeds();
        GeneratePrime();
        GenerateAB();
        BloomFilter b1(tempm, tempk, true);
        BloomFilter b2(tempm, tempk, false);
        
        unordered_set<int> subset;

        while (subset.size() < static_cast<size_t>(size)){
            num += numInterval;
            if (subset.insert(num).second) {
                b1.add(num);
                b2.add(num);
            }
        }

        map<int, float> b1TempMap = b1.getBinCounts();
        map<int, float> b2TempMap = b2.getBinCounts();
        for (const auto& bin : b1TempMap){
            b1TotalMap[bin.first] += bin.second;
        }
        
        for (const auto& bin : b2TempMap){
            b2TotalMap[bin.first] += bin.second;
        }
    }
    for (auto& key : b1TotalMap){
        key.second /= totalRuns;
    }
    for (auto& key : b2TotalMap){
        key.second /= totalRuns;
    }
    PopulateCsvFromMap("firstHash2.csv", b1TotalMap);
    PopulateCsvFromMap("secondHash2.csv", b2TotalMap);
}

void GetFalsePositiveRate(int numTestValues, int numIterPerTrial, bool hashF, string csvOut1, string csvOut2){
    map<int, vector<float>> tempMap10, tempMap15;
    map<int, float> medianMap10, medianMap15;
    int tempC = c;
    int tempM = 0;
    int tempK;

    for(int l = 0; l < 2; l++){
        for(float j = 0.4; j < 1.4; j += 0.1){
            tempK = round(j * tempC);
            for(int i = 0; i < numIterPerTrial; i++) {
                GenerateSeeds();
                GeneratePrime();
                GenerateAB();
                tempM = int(tempC * n);
                BloomFilter b(tempM, tempK, hashF);
                unordered_set<int> insertedSet = PopulateBloomFilter(b, n, N);

                unordered_set<int> notInsertedSet;
                random_device randomFP;
                mt19937 gen(randomFP());
                uniform_int_distribution<> world(0, N - 1);

                while (notInsertedSet.size() < static_cast<size_t>(numTestValues)){
                    int num = world(gen);
                    if (insertedSet.find(num) == insertedSet.end()) {
                        notInsertedSet.insert(num);
                    }
                }

                int falsePositives = 0;
                for (int x : notInsertedSet){
                    if(b.contains(x)){
                        falsePositives++;
                    }
                }
                if (tempC == 10){
                    tempMap10[tempK].push_back(static_cast<float>(falsePositives) / numTestValues);
                }
                else if (tempC == 15){
                    tempMap15[tempK].push_back(static_cast<float>(falsePositives) / numTestValues);
                }
                else {
                    cout << "OH NO" << endl;
                    return;
                }
            }
            cout << "Finished False Positive K: " << tempK << "  C: " << tempC << endl;
        }
        tempC = 15;
    }

    for (auto& pair : tempMap10){
        sort(pair.second.begin(), pair.second.end());
        int medianIndex = pair.second.size() / 2;
        medianMap10[pair.first] = pair.second[medianIndex];
    }

    for (auto& pair : tempMap15){
        sort(pair.second.begin(), pair.second.end());
        int medianIndex = pair.second.size() / 2;
        medianMap15[pair.first] = pair.second[medianIndex];
        cout << pair.first << ") ";
        for (float fp : pair.second){
            cout << fp << ",";
        }
        cout << " M: " << pair.second[medianIndex]<< endl;
    }

    PopulateCsvFromMap(csvOut1, medianMap10);
    PopulateCsvFromMap(csvOut2, medianMap15);

}

int main() {
    
    GenerateSeeds();
    GeneratePrime();
    GenerateAB();

    HashTestRandomInput(100, 1000000, 100, 100, 5, 100);
    HashTestLinearInput(100, 100, 5, 100, 100);
    
    
    auto startFunct1 = chrono::high_resolution_clock::now();
    GetFalsePositiveRate(n, 11, true, "falsePositive10H1.csv", "falsePositive15H1.csv");
    auto endFunct1 = chrono::high_resolution_clock::now();

    auto startFunct2 = chrono::high_resolution_clock::now();
    GetFalsePositiveRate(n, 11, false, "falsePositive10H2.csv", "falsePositive15H2.csv");
    auto endFunct2 = chrono::high_resolution_clock::now();

    auto durationFunct1 = chrono::duration_cast<std::chrono::milliseconds>(endFunct1 - startFunct1);
    auto durationFunct2 = chrono::duration_cast<std::chrono::milliseconds>(endFunct2 - startFunct2);

    cout << "Duration Hash Function 1 Test: " << durationFunct1.count() << endl;
    cout << "Duration Hash Function 2 Test: " << durationFunct2.count() << endl;

    cout << "End of Program" << endl;
    return 0;
    
}

