#include "test.cuh"

__attribute__((annotate("@critical_path(pointcut='around')")))
int main(int argc, char *argv[])
{
	Wrapper::wrapper();
}