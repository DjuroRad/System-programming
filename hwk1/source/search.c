#include "search.h"

void init(){
	formatted_text = (char*)malloc(sizeof(char));
	newpaths = (char**)malloc(sizeof(char**));

	if(formatted_text==NULL || newpaths==NULL){
		fprintf(stderr,"Malloc failed in init()");
		exit(EXIT_FAILURE);
	}
	formatted_text[0] = '\0';
	np=1;
	return;
}	

void searchDirRec(const char* dirpathname){
	init();

	add_to_text(dirpathname, false, 0, 0);
	if(!searchDirRec2(dirpathname, 1))
		add_to_text("No file with given attributes was found\n", false, 0, 0);

	return;
}

bool searchDirRec2(const char* dirpathname, unsigned int step){
	DIR* dir;
	struct dirent* sd;

	bool found = false;
	
	if( (dir = opendir(dirpathname)) == NULL){//open current dirrectory
		fprintf(stderr, "Error! Unable to open directory.\nDirectory name:%s\n", dirpathname);
		exit(1);
	}

	while( (sd=readdir(dir)) != NULL ){

		// print_formatted(sd->d_name, step);
		if( (strcmp(sd->d_name,".") != 0) && (strcmp(sd->d_name, "..") != 0) && checkFile( sd, dirpathname ) ){
			// printf("FILE IS FOUND!!!");
			found = true;
			add_to_text(sd->d_name, false, strlen(formatted_text), step);
		}
		/*recursive call here*/
		if( sd->d_name != NULL && sd->d_type == DT_DIR && (strcmp(sd->d_name,".") != 0) && (strcmp(sd->d_name, "..") != 0) ){
			char* new_pathname = getNewPath(dirpathname, sd->d_name);
			/*
				Reallocating new paths and updating the number of already made newpaths
			*/
			newpaths = (char**)realloc(newpaths, np*sizeof(char**));
			newpaths[np-1] = new_pathname;
			np++;

			// printf("Call to rec func: %s, %s, %s\n", dirpathname, sd->d_name, new_pathname);
			int prev_end = strlen(formatted_text);
			if( searchDirRec2( new_pathname, step+1 ) ){
				add_to_text(sd->d_name, true, prev_end, step);
				found = true;
			}
			free(new_pathname);
			--np;
		}
	}

	if( closedir(dir) ){
		fprintf(stderr, "Error while closing the directory\n");
		exit(EXIT_FAILURE);
	}
	return found;
}

bool checkFile( struct dirent* sd, const char* dirpathname){

	/*First two arguments are mandatory warg and farg*/
	bool found = true;

	/*Checking the -f name of the files considering the + regular expression*/
	int i=0, j=0;

	if(flgs[fflg]){
		while(args[fflg][i] != '\0' && sd->d_name[j] != '\0' && found){
			if( cmpChars( (sd->d_name)[j] , args[fflg][i] ) ){
				if(args[fflg][i+1] == '+'){//checking the regular expression and processing it
					++i;
					while( cmpChars(sd->d_name[j], sd->d_name[j+1]) )
						++j;
				}
				++i;
				++j;
			}
			else
				found = false;
		}
		if( !(args[fflg][i] == '\0' && sd->d_name[j] == '\0') ) found = false;

	}
	else found = false;

	if(found){
		struct stat statbuf;

		to_open = (char*)malloc( sizeof(char)*(strlen(dirpathname)+strlen(sd->d_name)+1+1 ) );
		to_open[strlen(dirpathname)+strlen(sd->d_name)+1] = '\0';
		strcpy(to_open, dirpathname);
		to_open[strlen(dirpathname)]='/';
		to_open[strlen(dirpathname)+1]='\0';
		strcat(to_open, sd->d_name);

		/*getting files stats*/
		if( stat( to_open, &statbuf) == -1 ){
			free(to_open);
			fprintf(stderr, "Error while calling stat() function, d_name:%s\n", to_open);
			exit(EXIT_FAILURE);
		}
		/*checking the -t type of the file*/
		if(found && flgs[tflg]){
			char type = args[tflg][0];
			switch(type){
				case 'd':
					if( (statbuf.st_mode & S_IFMT) != S_IFDIR )found = false;
					break;
				case 's':
					if( (statbuf.st_mode & S_IFMT) != S_IFSOCK )found = false;
					break;
				case 'b':
					if( (statbuf.st_mode & S_IFMT) != S_IFBLK )found = false;
					break;
				case 'c':
					if( (statbuf.st_mode & S_IFMT) != S_IFCHR )found = false;
					break;
				case 'f':
					if( (statbuf.st_mode & S_IFMT) != S_IFREG )found = false;
					break;
				case 'p':
					if( (statbuf.st_mode & S_IFMT) != S_IFIFO )found = false;
					break;
				case 'l':
					if( (statbuf.st_mode & S_IFMT) != S_IFLNK )found = false;
				break;
			}
		}

		/*Checking the -b, size of the file*/
		if(found && flgs[bflg] && (statbuf.st_size != atoi(args[bflg]) ) ) found = false;
		/*Checking the -l, number of hard links to a file*/
		if(found && flgs[lflg] && (statbuf.st_nlink!=atoi(args[lflg]) ) ) found = false;
		/*Checking the -p, permissions of a file*/
		if(found && flgs[pflg]){

			/*Check the permissions of the file*/
			if( !( ( args[pflg][0] == 'r' && (statbuf.st_mode & S_IRUSR) ) || 
				  (args[pflg][0] == '-' && !(statbuf.st_mode & S_IRUSR) ) )) found = false;
		
			if( !((args[pflg][1] == 'w' && (statbuf.st_mode & S_IWUSR)) || 
				(args[pflg][1] == '-' && !(statbuf.st_mode & S_IWUSR) ))) found = false;

			if( !((args[pflg][2] == 'x' && (statbuf.st_mode & S_IXUSR)) || 
				(args[pflg][2] == '-' && !(statbuf.st_mode & S_IXUSR) ))) found = false;

			//error checking
			// printf("-%d, %c, %o,%o, %s, %s\n", found, args[pflg][2], (statbuf.st_mode & S_IXUSR), statbuf.st_mode, sd->d_name, dirpathname );

			if( !((args[pflg][3] == 'r' && (statbuf.st_mode & S_IRGRP)) || 
				(args[pflg][3] == '-' && !(statbuf.st_mode & S_IRGRP) ))) found = false;

			if( !((args[pflg][4] == 'w' && (statbuf.st_mode & S_IWGRP)) || 
				(args[pflg][4] == '-' && !(statbuf.st_mode & S_IWGRP) ))) found = false;

			if( !((args[pflg][5] == 'x' && (statbuf.st_mode & S_IXGRP)) || 
				(args[pflg][5] == '-' && !(statbuf.st_mode & S_IXGRP) ))) found = false;

			if( !((args[pflg][6] == 'r' && (statbuf.st_mode & S_IROTH)) || 
				(args[pflg][6] == '-' && !(statbuf.st_mode & S_IROTH) ))) found = false;

			if( !((args[pflg][7] == 'w' && (statbuf.st_mode & S_IWOTH)) || 
				(args[pflg][7] == '-' && !(statbuf.st_mode & S_IWOTH) ))) found = false;

			if( !((args[pflg][8] == 'x' && (statbuf.st_mode & S_IXOTH)) || 
				(args[pflg][8] == '-' && !(statbuf.st_mode & S_IXOTH) ))) found = false;

		}
	}

	free(to_open);
	to_open = NULL;

	return found;
}

void free_allocated(){
	if(formatted_text!=NULL)
		free(formatted_text);
	int i;
	for( i=0; i<np-1; ++i){
		free( newpaths[i] );
	}
	
	if(newpaths!=NULL)
		free(newpaths);

	if(to_open!=NULL)
		free(to_open);

}