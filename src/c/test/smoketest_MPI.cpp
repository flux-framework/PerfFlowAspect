#include <iostream>
#include <math.h>
#include <mpi.h>

using namespace std;

#define MASTER  0       //main process

__attribute__((annotate("@critical_path(pointcut='around')")))
int foo(const string &str)
{
    printf("foo\n");
    int temp = 0;
    int temp1[10000] = {25};
    int temp2[10000] = {22};
    int temp3[10000] = {0};
    for (int i = 0; i < 10000; i++)
    {
        temp = temp + 1;
        temp3[i] = temp1[i] * temp2[i];
        for (int j = i + 1; j < 10000; j++)
        {
            temp3[j] = temp3[j] / 3;
        }
    }
    if (str == "hello")
        return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    int taskid, numProcs;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

    //main process code
    if (taskid == MASTER)
    {
        cout << "taskid: " << taskid << '\n';
        foo("hello");
    }

    //worker process code
    if (taskid != MASTER)
    {
        cout << "taskid: " << taskid << '\n';
        foo("hello");
    }

    MPI_Finalize();
}