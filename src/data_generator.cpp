#include<iostream>
#include<cstring>
#include<string>
#include<algorithm>
#include<stack>
#include<queue>
#include<cstdio>
#include<vector>
#include<cmath>
#include<sstream>
#include<random>
#include<ctime>
using namespace std;

#define N 100

int main(){
    srand((unsigned int)time(NULL));
    freopen("../test/matrix_input.txt", "w", stdout);
    printf("%d\n", N);
    for(int e=0; e<2; ++e){
        for(int i=0; i<N; ++i){
            for(int j=0; j<N; ++j){
                printf("%d ", rand()%10);
            }
            printf("\n");
        }
    }
    fclose(stdout);
    return 0;
}