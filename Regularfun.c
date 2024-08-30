#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <regex.h>
#include <sys/mman.h>  //映射头文件

int main(){
	char * regstr = "<a[^>]\\+\\?href=\"\\([^\"]\\+\\?\\)\"[^>]\\+\\?>\\([^<]\\+\\?\\)</a>";  //映射，将文件数据映射到内存
	int fd;
	fd = open("newslink",O_RDWR);
	int size = lseek(fd, 0, SEEK_END);

	char * data = NULL;
	data = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	close(fd);  //关闭描述符

	regex_t reg;
	int regnum = 3;
	regmatch_t match[regnum];

	regcomp(&reg, regstr, 0);

	char href[8192];
	char title[1024];

	while((regexec(&reg, data, regnum, match, 0)) == 0){
		//匹配成功
		bzero(href, sizeof(href));
		bzero(title, sizeof(title));
		snprintf(href, match[1].rm_eo - match[1].rm_so + 1, "%s", data+match[1].rm_so);
		snprintf(title, match[2].rm_eo - match[2].rm_so + 1, "%s", data+match[2].rm_so);
		printf("%s\t%s\n", href, title);
		data += match[0].rm_eo;
	}

	regfree(&reg);
	return 0;
}
