#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
	/*char * position = NULL;
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
	//}*/
	
	char command[100];
	int width;
	int height;
	int zoom;
	int page;
	char zoom_str[100];
	memset(zoom_str, '\0', sizeof(zoom_str));
	char page_str[100];
	memset(page_str, '\0', sizeof(page_str));
	char temp[100];
	scanf("%d %d %d %d", &width, &height, &zoom, &page);
	
	printf("%d %d\n", zoom, page);
	
	while(zoom>0){
		int a = zoom%10;
		sprintf(temp,"%d %s", a, zoom_str);
		strcpy(zoom_str, temp);
		zoom/=10;
		memset(temp ,'\0', sizeof(temp));
	}
	
	memset(temp ,'\0', sizeof(temp));
	while(page>0){
		int a = page%10;
		sprintf(temp,"%d %s", a, page_str);
		strcpy(page_str, temp);
		page/=10;
		memset(temp ,'\0', sizeof(temp));
	}
	
	printf("%s %s\n", zoom_str, page_str);
	
	sprintf(command, "xdotool search --onlyvisible --name okular windowactivate");
	system(command);
	sprintf(command, "xdotool getactivewindow windowsize %d %d", height, width);
	system(command);
	sprintf(command, "xdotool key --delay 250 ctrl+f Escape ctrl+f Tab Tab Tab Tab Tab %s Return Escape", zoom_str);
	system(command);
	sprintf(command, "xdotool key --delay 250 ctrl+g %s Return", page_str);
	system(command);
	/*sprintf(command, "xdotool search --onlyvisible --name okular windowactivate");
	system(command);
	sprintf(command, "xdotool getactivewindow windowsize 600 800");
	system(command);
	sprintf(command, "xdotool key --delay 250 ctrl+f Escape ctrl+f Tab Tab Tab Tab Tab 1 5 0 Return Escape");
	system(command);
	sprintf(command, "xdotool key --delay 250 ctrl+g 1 2 Return");
	system(command);*/
	return 0;
}
