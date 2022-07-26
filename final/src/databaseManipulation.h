#include "utils.h"


typedef struct Cell{
	char* value;
	struct Cell* next;/*going downards*/
}Cell;

typedef struct Column{
	char* attr_name;
	Cell* cells;
	struct Column* next_attr;
}Column;

typedef struct Table{
	Column* col_first;
	int attr_n;
}Table;

void initDB();
void readCSV(const char* path_name);
void printColumnNames();
void addIdColumn();
void freeTable();
void SELECT( char* query );
void SELECT_DISTINCT( char* query );
double getExecutionTime();
int getRowCount();
void QuerySQL( char* query );
char** getResCopy();
/*there is a list of column attributes represented with Data*/
