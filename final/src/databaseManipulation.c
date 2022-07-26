#include "databaseManipulation.h"
#include "string.h"
#include "types.h"

#define BUFF_SIZE 256

Table table = {
	.col_first = NULL,
	.attr_n = 0
};

int fd = -1;
int fd_col_read = -1;

clock_t begin;
clock_t end;
double time_spent;

void readColumn(const char* path_name);
void printTable();
void printColumn(Column* col);
void clear_DB_Allocs();
void addToResponse(const char* new_value, int row);
void clearResponse();
void printResponse();

char** response = NULL;
int response_row_count = 0;

char** to_update = NULL;
int size =0;
void freeToUpdate();
void clear_handler();

void printResponse(){
	int i;
	for( i = 0; i< response_row_count; ++i )
		printf("%s", response[i]);
}
void printTable(){
	// printf("PRINTING THE TABLE!!!\n");
	Column* col_curr = table.col_first;
	while( col_curr != NULL ){
		addToResponse(col_curr->attr_name, 0);
		printColumn(col_curr);
		col_curr = col_curr->next_attr;
	}

	return;
}

void addToResponse(const char* new_value, int row){
	bool flg = false;
	// printf("%d - %d IN HERE %s - %ld\n", response_row_count, row,new_value, strlen(new_value));
	if( response == NULL ){
		response = (char**)malloc_util(sizeof(char**));
		++response_row_count;
		flg = true;
	}
	else if( response_row_count <= row ){
		
		++response_row_count;
		response = (char**)realloc_util(response, response_row_count*sizeof(char**));
		response[response_row_count-1] = NULL;
		// printf("Adding new row of size %d\n", response_row_count);

		flg = true;
	}

	int len_prev = 0;
	if( !flg ){
		len_prev = strlen(response[row]);
		response[row] = (char*)realloc_util(response[row], len_prev + strlen(new_value) + 1 + 1 );
		response[row][ len_prev -1 ] = '\t';
	}
	else{
		response[row] = (char*)calloc(len_prev+strlen(new_value)+2, sizeof(char) );
	}
	
	if(flg){
		strcpy(response[row], new_value);
	}
	else
		strcat(response[row], new_value);

	response[row][len_prev+strlen(new_value)] = '\n';
	response[row][len_prev+strlen(new_value)+1] = '\0';	

}

void printColumn(Column* col){
	Cell* cell = col->cells;
	int row  = 1;/*at the 0th level there will always be the names of the attributes*/
	while( cell != NULL ){
		addToResponse(cell->value, row);
		// printf("%s\n", cell->value);
		cell = cell->next;
		row++;
	}
}

void initDB(){
	
	table.col_first = NULL;
	table.attr_n = 0;

	table.col_first = (Column*)malloc_util(sizeof(Column));

	// table.col_first->attr_name = (char*)malloc_util(sizeof(char));
	// table.col_first->attr_name[0] = '\0';

	return;
}

void readCSV(const char* path_name){
	begin = clock();

	fd = open_util( path_name, O_RDONLY );
	/*first make the linked list for each of the columns*/
	// addIdColumn();
	// readColumn(path_name);
	Column* temp = table.col_first;
	temp -> attr_name = (char*)malloc_util(sizeof(char));
	temp -> attr_name[0] = '\0';

	int temp_i = 0;
	char new_char;
	read_util(fd, &new_char, sizeof(new_char));
	while( new_char!='\n' ){
		// printf("NEW CHAR %c\n", new_char);
		/*now add the buffer char by char here*/
		if( new_char != ',' && new_char != ';' && new_char != '\n' && new_char != '\0'){

			temp -> attr_name = realloc_util( temp->attr_name, sizeof(char)*(strlen(temp->attr_name)+2) );
			temp -> attr_name [strlen(temp->attr_name)+1] = '\0';
			temp -> attr_name[temp_i++] = new_char;
		}
		
		read_util(fd, &new_char, sizeof(new_char));
		
		if( new_char == ',' || new_char=='\n'){
			/*load the whole column now in here!*/
			// printf("NOW COLUMNS FOR %s\nRet val %dRead char %c - %d\n\n\n", temp->attr_name, ret_val, new_char, new_char);

			readColumn(path_name);

			if( new_char != '\n' ){
				temp -> next_attr = (Column*)malloc_util(sizeof(Column));
				temp = temp->next_attr;
				temp -> attr_name = (char*)malloc_util(sizeof(char));
				temp -> attr_name[0] = '\0';
				temp -> next_attr = NULL;
				temp -> cells = NULL;
				temp_i = 0;
			}
		}
	}

	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

}

void addIdColumn(){
	char str[] = "ID";
	table.col_first->attr_name = (char*)malloc_util(sizeof(char)*sizeof(str));
	strcpy(table.col_first->attr_name, str);
	// table.attr_n++;

	table.col_first->next_attr = (Column*)malloc_util(sizeof(Column));
	table.col_first->cells = NULL;
	table.col_first->next_attr->attr_name = (char*)malloc_util(sizeof(char));
	table.col_first->next_attr->attr_name[0] = '\0';
	return;
}
void readColumn(const char* path_name){

	fd_col_read = open_util( path_name, O_RDONLY );

	char new_char;
	do{
		read_util(fd_col_read, &new_char, sizeof(new_char));
	}while(new_char != '\n');
	/*go through the first row since it is immutable*/

	Column* temp = table.col_first;


	int comma_count = 0;
	while(comma_count < table.attr_n){
		read_util(fd_col_read, &new_char, sizeof(new_char));/*moving the file descriptor*/
		if( new_char == ',' || new_char == ';' ){
			++comma_count;
			temp = temp -> next_attr;
		}
	}
	/*we reached the column here!*/

	/*now we read the value of this cell and store it!*/
	temp -> cells = (Cell*)malloc_util(sizeof(Cell));
	Cell* new_cell = temp -> cells;
	new_cell->value = (char*)malloc_util(sizeof(char));
	new_cell->value[0] = '\0';
	new_cell -> next = NULL;

	int read_val = 1;
	do{
		/*now add one by one each column's value*/
		int temp_i = 0;
		read_val = read_util(fd_col_read, &new_char, sizeof(new_char));/*moving the file descriptor*/
		while( new_char != ',' && new_char != '\n' && read_val){
			new_cell -> value = realloc_util( new_cell -> value, strlen(new_cell->value)+2);
			new_cell->value[strlen(new_cell->value)+1] = '\0';
			new_cell->value[temp_i++] = new_char;
			read_val = read_util(fd_col_read, &new_char, sizeof(new_char));/*moving the file descriptor*/
		}
		/*now iterate to the next column*/
		while( new_char != '\n'){
			if( (read_val=read_util(fd_col_read, &new_char, sizeof(new_char))) == 0 )/*moving the file descriptor*/
				break;
		}

		if(read_val){/*means end of file is not reached yet!!! ---> more attributes*/
			// printf("Value added: %s\n", new_cell->value);
			
			/*move this again for the new column to be read*/
			comma_count = 0;
			while(comma_count < table.attr_n && read_val){
				read_val = read_util(fd_col_read, &new_char, sizeof(new_char));/*moving the file descriptor*/
				// printf("%c - %d - %d \n", new_char, new_char, table.attr_n);
				if( new_char == ',' || new_char == ';' ){
					++comma_count;
				}
			}
			if( read_val ){
				new_cell -> next = (Cell*)malloc_util(sizeof(Cell));
				new_cell = new_cell->next;
				new_cell->value = (char*)malloc_util(sizeof(char));
				new_cell->value[0] = '\0';
				new_cell->next = NULL;
			}
		}
	}while(read_val);

	table.attr_n = table.attr_n + 1;
	return;
}
// char* attr_name;
// Cell* cells;
// struct Column* next_attr;
void freeColumn(Column* col){
	if( col -> attr_name != NULL )
		free(col->attr_name);

	/*now remove the cells and its values*/
	if(col -> cells != NULL ){
		Cell* cells = col->cells;
		while(cells != NULL){
			Cell* to_free = cells;
			cells = cells->next;
			if( to_free->value != NULL)
				free(to_free->value);
			free(to_free);
		}
	}
}

void freeTable(){
	Column* col = table.col_first;
	while(col!=NULL){
		freeColumn(col);
		Column* temp = col;
		col = col->next_attr;
		free(temp);
	}
	table.col_first = NULL;
}


void printColumnQuery( char* target_name ){
	Column* temp = table.col_first;
	// printf("TARGET NAME IS %s\n", target_name);
	while( strcmp(temp->attr_name, target_name) != 0 ){
		// printf("HERE %s\n\n", temp->attr_name);
		temp = temp->next_attr;
	}
	addToResponse(temp->attr_name,0);
	printColumn(temp);

	return;
}
void SELECT( char* query ){
	// printf("QUERY IS %s\n", query);
	char* new_str;

	new_str = strtok(query, " ");
	char query_keyword[] = "SELECT";

	while( strcmp(new_str, query_keyword) != 0 )
		new_str = strtok( NULL, " " );

	/* now we reached the part after which the SELECT is passed and everything else will be column names etc. */
	while( (new_str = strtok(NULL,", ")) != NULL ){
		// printf("New string is %s\n", new_str);
		if( strcmp(new_str, "FROM") == 0 )
			break;
		if( strcmp(new_str, "*") == 0 ){
			printTable();
			break;
		}
		// printf("CALLING WITH %s\n", new_str);

		printColumnQuery( new_str );
	}
	
}

bool checkDuplicateCell( Cell* curr_cell ){
	bool found = false;
	char* org_value = curr_cell->value;
	
	curr_cell = curr_cell->next;
	
	while( !found && curr_cell != NULL ){

		if( strcmp( org_value, curr_cell->value) == 0 )
			found = true;
		curr_cell = curr_cell->next;	
	}

	return found;
}

void printColumnDistinct( char* new_str ){
	Column* col_print = table.col_first;
	while( strcmp( col_print->attr_name, new_str ) != 0 )
		col_print = col_print->next_attr;
	addToResponse(col_print->attr_name, 0);
	
	Cell* cell_print = col_print->cells;
	int row = 1;
	while( cell_print != NULL ){
		if( !checkDuplicateCell( cell_print ) ){
			addToResponse(cell_print->value, row);
			// printf("adding %s %d \n", cell_print->value, row);
			++row;
		}
		cell_print = cell_print->next;
			
	}

}

void SELECT_DISTINCT( char* query ){
	// printf("QUERY IS %s\n", query);
	char* new_str;
	new_str = strtok(query, " ");
	new_str = strtok(NULL, " ");
	/*now both SELECT and DISTINCT are returned and the pointer is at the first column that needs to be processed*/
	while( (new_str = strtok(NULL, ", ")) != NULL ){
		if( strcmp("FROM", new_str) == 0 )
			break;
		
		printColumnDistinct( new_str );

	}

}

Column* getColumn(char* column_name){
	Column* target = table.col_first;
	while( strcmp(target->attr_name, column_name) != 0 )
		target = target->next_attr;
	
	return target;
}

int ret_index_getIndex = 0;
Cell* current_cell_getIndex = NULL;

int getIndex(Column* target_column, char* target_value){
	if( target_column != NULL )
		current_cell_getIndex = target_column->cells;
	else
		ret_index_getIndex++;

	while( current_cell_getIndex != NULL ){
		if( strcmp(current_cell_getIndex->value, target_value) == 0 ){
			current_cell_getIndex = current_cell_getIndex->next;

			return ret_index_getIndex;
		}
		++ret_index_getIndex;
		current_cell_getIndex = current_cell_getIndex->next;
	}

	ret_index_getIndex = 0;
	current_cell_getIndex = NULL;
	return -1;
}

char* column_name_update = NULL;
char* new_value_update = NULL;
void updateForReal(int index){
	int i;
	for( i = 0; i<size; ++i ){

		if( column_name_update == NULL )
			column_name_update = strtok(to_update[i], "=");
		Column* col = getColumn(column_name_update);
		Cell* cell_to_update = col->cells;
		int j =0;
		for( j=0; j<index; ++j)
			cell_to_update = cell_to_update->next;
		free(cell_to_update->value);
		if( new_value_update == NULL )
			new_value_update = strtok(NULL, "';");
		cell_to_update->value = (char*)malloc_util(sizeof(char)*(strlen(new_value_update)+1));
		strcpy(cell_to_update->value, new_value_update);
	}
}

char* target = NULL;
void updateHelper(){
	/*now all the attributes to be updated are in the to_update dynamic array of strings*/
	/*let's now find the index or the per se index of the column to be updated*/
	/*since strtok saves our string to static memory we can continue where we left of*/
	char* new_to_add = NULL;
	while( (new_to_add = strtok(NULL, " ") )!= NULL ){
		int last = 0;
		if( target != NULL )
			last = strlen(target);
		target = (char*)calloc(sizeof(char)*(last+strlen(new_to_add) + 2 ), sizeof(char)  );
		if( last != 0 )
			target[last] = ' ';
		strcat(target, new_to_add);

	}
	// printf("Target is %s\n", target);
	strtok(target, "=");
	// printf("Column name is %s\n", column_name);
	Column* column_target = getColumn(target);
	char* target_value = strtok(NULL, "';");
	// printf("Target value is %s\n", target_value);
	int index = getIndex(column_target, target_value);/*there can be multiple columns with such values*/
	// printf("index is %d\n", index);
	while( index != -1 ){
		updateForReal(index);
		index = getIndex(NULL, target_value);

	}
	column_name_update = NULL;
	new_value_update = NULL;
	free(target);
	target = NULL;
	// char str[] = " columnName='vlauex';";
	// char* new = strtok(str, " ");
	// printf("%s",new);
	// printf("%s\n", strtok(new, "="));/*gets the name of the column*/
	// printf("%s\n", strtok(NULL, "';"));/*gets the value in the column*/

}


void UPDATE( char* query ){

	/*iterate through UPDATE TABLE SET to get TO attributes*/
	strtok(query, " ");
	strtok(NULL, " ");
	strtok(NULL, " ");
	/*NOW WE ARE HERE AT THE command we need to execute*/
	/*let's get the attributes that are needed here*/
	/*multiple attributes will be here of course*/
	/*i will first find the first instance of this attribute and than while checking for it in other columns also do the same thing*/
	char* atribut = strtok(NULL, " ");
	/*check until "WHERE" is reached after which we will take */

	/*make a dynamic array of columns to be updated*/
	to_update = (char**)malloc_util(sizeof(char**));

	bool concat_flag = false;

	while( strcmp( atribut, "WHERE") != 0 ){
		// printf("ATRIBUT IS %s\n", atribut);
		if( !concat_flag ){
			to_update[size] = (char*)malloc_util(sizeof(char)*(strlen(atribut)+1));
			strcpy(to_update[size], atribut);/*copy it now here so that we can store it*/
			if( to_update[size][strlen(to_update[size])-1] != '\'')
				concat_flag = true;
			// printf("NEW ARG IS %s and 0 is %c and last is %c \n", to_update[size], to_update[size][0], to_update[size][strlen(to_update[size]) -1 ]);
			size++;
			to_update = (char**)realloc_util(to_update, (size+1)*sizeof(char**) );
		}
		else{
			// printf("WER ARE IN HERE\n");
			int last = strlen(to_update[size-1]);
			to_update[size-1] = (char*)realloc_util(to_update[size-1],sizeof(char)*(strlen(atribut)+strlen(to_update[size-1])+2));
			to_update[size-1][last] = ' ';
			strcat(to_update[size-1], atribut);
			if(to_update[size-1][strlen(to_update[size-1])-1] == '\'')
				concat_flag = false;
		}
		atribut = strtok(NULL, ", ");
	}
	// printf("Size is %d\n", size);
	// for(int i =0; i<size; ++i )
		// printf("%s\n",to_update[i]);
	updateHelper();

	freeToUpdate();
	return;
}
void QuerySQL( char* query ){
	begin = clock();

	char select[] = "SELECT";
	char distinct[] = "DISTINCT";
	/*Three possible cases, it is either SELECT, SELECT DISTINCT or UPDATE*/
	bool found = true;
	int i;
	for( i = 0; found && i<(int)strlen(select); ++i )
		if( query[i] != select[i] )
			found = false;

	if( found ){
		/*now it's either SELECT or SELECT DISTINCT*/
		while(query[i] == ' ')
			++i;

		for( int j = 0; found && (j < (int)strlen(distinct)); ++i, ++j){
			if( query[i] != distinct[j] )
				found = false;
		}

		if( found )
			SELECT_DISTINCT(query);
		else
			SELECT(query);
	}
	else{
		/*otherwise it can only be update query*/
		UPDATE(query);
	}

	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
}

void freeToUpdate(){
	int i;

	if( to_update != NULL ){
		for( i=0; i<size; ++i ){
			if(to_update[i] != NULL)
				free(to_update[i]);
		}
		free(to_update);
	}

	to_update = NULL;
	size = 0;
}

void clear_DB_Allocs(){
	if( current_cell_getIndex != NULL )
		free(current_cell_getIndex);
	if( column_name_update != NULL )
		free(column_name_update);
	if( new_value_update != NULL )
		free(new_value_update);
	if( target != NULL )
		free(target);
}
void clearResponse(){
	int i;
	for( i = 0; i<response_row_count; ++i ){
		free( response[i] );
		response[i] = NULL;
	}

	free(response);
	response = NULL;
	response_row_count = 0;
}
void clear_handler(){
	freeTable();
	freeToUpdate();
	clear_DB_Allocs();
	clearResponse();
}

double getExecutionTime(){
	return time_spent;
}

char** getResCopy(){
	char** dest;
	dest = malloc( response_row_count * sizeof(char*) );
	for( int i = 0; i<response_row_count; ++i ){
		dest[i] = (char*)malloc_util((strlen(response[i])+1)*sizeof(char));
		memcpy( dest[i], response[i], strlen(response[i]+1 ) );
	}

	return dest;
}

int getRowCount(){
	return response_row_count;
}