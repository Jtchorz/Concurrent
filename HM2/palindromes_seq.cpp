#include <stdio.h>
#include <omp.h>
#include <fstream>
#include <algorithm>
#include <vector>

#define MAXSIZE 235886  /* maximum matrix size */
#define MAXWORKERS 16   /* maximum number of workers */


using namespace std;

double start_time, end_time;

vector<string> check, dict;

int numWorkers;
int numLines;


int main(int argc, char *argv[]) {
    numLines = (argc > 1)? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
    if (numLines> MAXSIZE) numLines = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

 

    ifstream f("/usr/share/dict/words");

    if (!f.is_open()) {
        printf("Error opening the file!");
        return 1;
    }
    string s;
    int i = 0;

    while (getline(f, s)){
        if (numLines > i){  //I assume that even with a smaller dictionary we will want to check against the whole thing
            check.push_back(s);
            i++;
        }
        dict.push_back(s);
    }
    f.close();

  sort(dict.begin(), dict.end());

    int size = check.size();
    auto begin = dict.begin();
    auto end = dict.end();

    start_time = omp_get_wtime();
    


    vector<string> result;
    for (i = 0; i < size; i++){
        s = check[i];
        string rev = "";
        for(int j = s.size()-1; j>=0; j--)
            rev += s[i];
            
        if(binary_search(begin, end, rev))
            result.push_back(s);
    }

    end_time = omp_get_wtime();
 //   for(int i = 0; i < result.size(); i++)
   //     printf("%s \n", result[i].c_str());

    printf("it took %g seconds\n", end_time - start_time);
    return 0;
}
