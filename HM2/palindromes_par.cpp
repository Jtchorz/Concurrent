#include <stdio.h>
#include <omp.h>
#include <fstream>
#include <algorithm>
#include <vector>

#define MAXSIZE 235886  /* maximum matrix size */
#define MAXWORKERS 32   /* maximum number of workers */


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

    omp_set_dynamic( 0 );
    omp_set_num_threads( numWorkers );

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
    const auto begin = dict.begin();
    const auto end = dict.end();
    vector<string> global_result;

    start_time = omp_get_wtime();


#pragma omp parallel if(size >= 1000)
{
    string st;
    vector<string> result;
    int j, k;

    #pragma omp for nowait schedule(static)
    //ftom here
    for (j = 0; j < size; j++){
        st = check[j];
        string rev = "";
        for(k = st.size()-1; k>=0; k--)
            rev += st[k];

                    
        if(binary_search(begin, end, rev))
            result.push_back(st);
    }
//to here
#pragma omp critical
{
    global_result.insert(global_result.end(), result.begin(), result.end());
}
//also here
}

    end_time = omp_get_wtime();

    ofstream f2("output.txt");

    if (!f2.is_open()) {
        printf("Error opening the file!");
        return 1;
    }

    for(int i = 0; i < global_result.size(); i++)
        f2 << global_result[i] << '\n';


  // for(int i = 0; i < global_result.size(); i++)
    //    printf("%s \n", global_result[i].c_str());

    printf("it took %g seconds and %d \n", end_time - start_time, i);

}