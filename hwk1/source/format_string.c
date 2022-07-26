#include "format_string.h"

void add_to_text(const char* to_append, bool is_dir, int start, int step){
	
	char text_to_add[step*2+1+strlen(to_append)+1];/*1 for | character and the 1 on the end for \n character*/
	text_to_add[0]='|';

	int id;
	for(id=1; id<step*2+1; ++id) text_to_add[id] = '-';
	text_to_add[id]='\0';
	strcat(text_to_add, to_append);
	text_to_add[ step*2+1+strlen(to_append) ]='\n';
	text_to_add[ step*2+1+strlen(to_append)+1 ]='\0';

	// printf("\nBefore Adding\nText is: %s\nLenght: %ld\n", text_to_add, strlen(text_to_add));
	// printf("Lenght of the formatted string: %d, lenght of the new formatted string %d\n", strlen(formatted_text),strlen(formatted_text)+strlen(text_to_add)+1 );
	int end_prev = strlen(formatted_text);
	int new_length = strlen(formatted_text)+strlen(text_to_add);
	formatted_text = realloc(formatted_text, new_length+1);

	if(formatted_text==NULL){
		fprintf(stderr,"Realloc wasn't able to allocate memory for the path\nInside add_to_text function\n");
		exit(EXIT_FAILURE);
	}
	formatted_text[new_length] = '\0';
			
	if(is_dir){/*When adding a directory it needs to be added at the begining*/
		// printf("end_prev: %d, start: %d \n", end_prev, start);
		int i_add;
		for(i_add = 0; i_add<strlen(text_to_add); ++start, ++i_add, ++end_prev){/*for letters to be inserted*/

			int j;
			for(j=end_prev; j>start; --j)
				formatted_text[j] = formatted_text[j-1];
			formatted_text[start] = text_to_add[i_add];
			// printf("start: %d, i_add:%d ", start, i_add);
		}
	}
	else{
			/*when adding a file*/
			int i = 0;
			for(i=0; i<strlen(text_to_add); ++i, ++end_prev){
				formatted_text[end_prev] = text_to_add[i];
			}
		}

	// printf("Text is: %s\nStrlen is %ld\n", formatted_text, strlen(formatted_text));
	return;
}

bool cmpChars(char c1, char c2){
	
	if( c1==c2 || c1 == (c2+32) || c1==(c2-32) )
		return true;
	else 
		return false;

}

/*After calling this function, it is needed to free the character pointer returned*/
char* getNewPath(const char* dirpathname,const char* d_name){
	char* new_path = (char*)malloc( sizeof(char)*(strlen(dirpathname) + 1 + strlen(d_name) + 1) );
	strcpy(new_path, dirpathname);
	strcat(new_path, "/");
	strcat(new_path, d_name);
	// printf("S1: %s S2: %s S3: %s \n", dirpathname, d_name, new_path);
	// printf("L1: %d L2: %d L3: %d \n", strlen(dirpathname), strlen(d_name), strlen(new_path) );
	return new_path;
}

void print_formatted(char* d_name, unsigned int step){
	
	printf("|");
	int i;
	for(i=0; i<step*2; ++i)
		printf("-");
	printf("%s\n", d_name);
	return;
}