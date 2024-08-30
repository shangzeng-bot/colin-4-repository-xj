#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <regex.h>
#include <sys/mman.h>  // 映射头文件

int main() {
    int fd;
    fd = open("newlist.txt", O_RDONLY);  // 打开文件
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    int size = lseek(fd, 0, SEEK_END);  // 获取文件大小
    if (size == -1) {
        perror("lseek");
        close(fd);
        return EXIT_FAILURE;
    }

    char *data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);  // 映射文件到内存
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }
    close(fd);  // 关闭文件描述符

    // 定义正则表达式
    char *link_regex = "<a[^>]*href=\"([^\"]*)\"[^>]*>([^<]*)</a>";
    char *img_regex = "<img[^>]*src=\"([^\"]*)\"[^>]*>";
    char *title_regex = "(\\w+:\\s)?([^\\s]+)\\s(.*)";  // 假设新闻名在冒号和日期之后

    regex_t reg_link, reg_img, reg_title;
    regcomp(&reg_link, link_regex, REG_EXTENDED);
    regcomp(&reg_img, img_regex, REG_EXTENDED);
    regcomp(&reg_title, title_regex, REG_EXTENDED);

    regmatch_t match_link[3];
    regmatch_t match_img[2];
    regmatch_t match_title[3];

    char href[1024];
    char title[1024];
    char img_src[1024];

    while (regexec(&reg_link, data, 3, match_link, 0) == 0) {
        // 提取网页链接
        snprintf(href, sizeof(href), "%s", data + match_link[1].rm_so);
        printf("网页链接: %s\n", href);
        data += match_link[0].rm_eo;
    }

    data = data - strlen(data) + size;  // 重置指针到文件开始

    while (regexec(&reg_img, data, 2, match_img, 0) == 0) {
        // 提取图片链接
        snprintf(img_src, sizeof(img_src), "%s", data + match_img[1].rm_so);
        printf("图片链接: %s\n", img_src);
        data += match_img[0].rm_eo;
    }

    data = data - strlen(data) + size;  // 重置指针到文件开始

    while (regexec(&reg_title, data, 3, match_title, 0) == 0) {
        // 提取新闻名
        snprintf(title, sizeof(title), "%s", data + match_title[2].rm_so);
        printf("新闻名: %s\n", title);
        data += match_title[0].rm_eo;
    }

    // 释放正则表达式
    regfree(&reg_link);
    regfree(&reg_img);
    regfree(&reg_title);
    
    // 取消映射
    munmap(data, size);

    return 0;
}
