#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]){
	char * position = NULL;
	char * size = NULL;
	char * zoom = NULL;
	char * page = NULL;
	char * msg = NULL;
	size_t len = 0;
	ssize_t read;
	//while(1){
		FILE* fp;
		fp = fopen("position.txt", "w");
		fclose(fp);
		fp = fopen("size.txt", "w");
		fclose(fp);
		fp = fopen("page_no.txt", "w");
		fclose(fp);
		fp = fopen("zoom.txt", "w");
		fclose(fp);
		usleep(100);
		system("./get");
		fp = fopen("position.txt", "r");
		getline(&position, &len, fp);
		fclose(fp);
		fp = fopen("size.txt", "r");
		getline(&size, &len, fp);
		fclose(fp);
		fp = fopen("page_no.txt", "r");
		getline(&page, &len, fp);
		fclose(fp);
		fp = fopen("zoom.txt", "w");
		geltine(&zoom, &len, fp);
		fclose(fp);
		sprintf(msg, "%s %s %s %s\n", position, size, page, zoom);
		printf("%s", msg);
	//}
	return 0;
}
