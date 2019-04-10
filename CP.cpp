#include <iostream>
#include <string>
#include <dirent.h>
#include <errno.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <fstream>

using namespace std;

int Copy_Files(const char *src, const char *dst, bool force);
void List_Files(string baseDir, bool recursive, char *dstDir, bool force);
void error_mes(const char *s1, const char *s2);
bool dirExists(const char *path);
bool isReg(string dir);
bool isDir(string dir);



struct CP
{
    bool force = false;
    bool recursive = false;
};



int main(int argc, char *argv[])
{

    const char *short_options = "hrf";

    const struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"recursive", no_argument, NULL, 'r'},
        {"force", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}};

    int rez;
    int option_index;

    CP *obj = new CP();

    while ((rez = getopt_long(argc, argv, short_options,
                              long_options, &option_index)) != -1)
    {
        switch (rez)
        {
        case 'h':
        {
            printf("Страница помощи.\n");
            const char fname[36] = "help.file";
            string line;
            ifstream fin(fname, ios::in);

            while (getline(fin, line))
            {
                cout << line << endl;
            }
            fin.close();
            break;
        };
        case 'r':
        {
            obj->recursive = true;
            break;
        };

        case 'f':
        {
            obj->force = true;
            break;
        };
        case '?':
        default:
        {
            printf("Такого ключа нет. Попробуйте -h or --help. \n");
            break;
        };
        };
    }

    // char slash=  "/";
    char infst = argv[argc - 2][strlen(argv[argc - 2]) - 1];
    char inScd = argv[argc - 1][strlen(argv[argc - 1]) - 1];

    if (argc > 2)
    {

        if (infst != '/' && inScd != '/')
        {
            // Если оба файла то копируем Файл1 в Файл2
            Copy_Files(argv[argc - 2], argv[argc - 1], obj->force);
        }
        else if (infst != '/' && inScd == '/')
        {
            // Если 1 файл, а второй каталог, то проверка на сущ. каталога
            if (dirExists(argv[argc - 1]))
            {
                char tmpSrc[strlen(argv[argc - 2]) + 1];
                strcpy(tmpSrc, argv[argc - 2]);
                char *p = strtok(tmpSrc, "/");
                char *name;

                while (p != NULL)
                {
                    name = p;
                    p = strtok(NULL, "/");
                }
                strcat(argv[argc - 1], name);
                Copy_Files(argv[argc - 2], argv[argc - 1], obj->force);
            }
            else
            {
                mkdir(argv[argc - 1], 0755);
                char tmpSrc[strlen(argv[argc - 2]) + 1];
                strcpy(tmpSrc, argv[argc - 2]);
                char *p = strtok(tmpSrc, "/");
                char *name;

                while (p != NULL)
                {
                    name = p;
                    p = strtok(NULL, "/");
                }
                strcat(argv[argc - 1], name);
                Copy_Files(argv[argc - 2], argv[argc - 1], obj->force);
            }
        }
        else if (infst == '/' && inScd != '/')
        {
            cout << "Копирование каталога в файл невозможно. См. ./CP -h или ./CP --help для справки" << endl;
        }
        else if (infst == '/' && inScd == '/')
        {
            // Если оба каталога и введен ключ рекурсии то коипруем каталоги рекурсивно
            if (obj->recursive)
            {
                if (dirExists(argv[argc - 2]))
                {
                    if (dirExists(argv[argc - 1]))
                    {
                        List_Files(argv[argc - 2], obj->recursive, argv[argc - 1], obj->force);
                    }
                    else
                    {
                        mkdir(argv[argc - 1], 0755);
                        List_Files(argv[argc - 2], obj->recursive, argv[argc - 1], obj->force);
                    }
                }
                else
                {
                    cout << "Исходного каталога не существует. См. ./CP -h или ./CP --help для справки" << endl;
                }
            }
            else
            {
                cout << "Копирование каталога в каталог невозможно без ключа -r или --recursive. См. ./CP -h или ./CP --help для справки" << endl;
            }
        }
    }

    delete obj;

    return 0;
}



bool dirExists(const char *path)
{
    struct stat sb;

    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isReg(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISREG(fileInfo.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isDir(string dir)
{
    struct stat fileInfo;
    stat(dir.c_str(), &fileInfo);
    if (S_ISDIR(fileInfo.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void List_Files(string baseDir, bool recursive, char *dstDir, bool force)
{
    DIR *dp;
    struct dirent *dirp;

    char tmpDst[strlen(dstDir) + 1];
    char tmpSrc[strlen(baseDir.c_str()) + 1];

    if ((dp = opendir(baseDir.c_str())) == NULL)
    {
        cout << "[ОШИБКА: " << errno << " ] Невозможно открыть каталог " << baseDir << "." << endl;
        return;
    }
    else
    {
        while ((dirp = readdir(dp)) != NULL)
        {
            if (dirp->d_name != string(".") && dirp->d_name != string(".."))
            {
                if (isDir(baseDir + dirp->d_name) == true && recursive == true)
                {
                    strcpy(tmpDst, dstDir);
                    strcat(tmpDst, "/");
                    strcat(tmpDst, dirp->d_name);
                    mkdir(tmpDst, 0755);
                    List_Files(baseDir + dirp->d_name + "/", recursive, tmpDst, force);
                }
                else if (recursive == false)
                {
                    cout << "Без флага -r копирование каталога запрещено. Смотрите ./CP -h\n";
                }
            }
        }
        closedir(dp);
        dp = opendir(baseDir.c_str());

        char tmpDst[strlen(dstDir) + 1];
        char tmpSrc[strlen(baseDir.c_str()) + 1];
        while ((dirp = readdir(dp)) != NULL)
        {
            if (dirp->d_name != string(".") && dirp->d_name != string(".."))
            {
                if (isDir(baseDir + dirp->d_name) == true && recursive == true)
                {
                    strcpy(tmpDst, dstDir);
                    strcat(tmpDst, "/");
                    strcat(tmpDst, dirp->d_name);
                    List_Files(baseDir + dirp->d_name + "/", recursive, tmpDst, force);
                }
                else
                {
                    strcpy(tmpDst, dstDir);
                    strcat(tmpDst, "/");
                    strcat(tmpDst, dirp->d_name);
                    string tmp = baseDir + "/" + dirp->d_name;
                    Copy_Files(tmp.c_str(), tmpDst, force);
                }
            }
        }
        closedir(dp);
    }
}

void error_mes(const char *s1, const char *s2)
{
    fprintf(stderr, "Ошибка: %s ", s1);
    perror(s2);
    exit(1);
}

int Copy_Files(const char *src, const char *dst, bool force = false)
{
    int in_file, out_file, n_chars, fd;
    char buf[1024];
    if ((in_file = open(src, O_RDONLY)) == -1)
    {
        error_mes("Невозможно открыть исходный файл. ", src);
    }
    if (((out_file = creat(dst, 0755)) == -1) && !force)
    {
        error_mes("Невозможно открыть/создать файл назначения. Воспользуйтесь ключом -f. Смотрите ./CP -h \n", dst);
    }
    else if (((out_file = creat(dst, 0755)) == -1) && force)
    {
        chmod(dst, 0755);
        out_file = creat(dst, 0755);
    }

    while ((n_chars = read(in_file, buf, 1024)) > 0)
    {
        if (write(out_file, buf, n_chars) != n_chars)
        {
            error_mes("Ошибка записи в файл ", dst);
        }

        if (n_chars == -1)
        {
            error_mes("Ошибка чтения из ", src);
        }
    }

    /* close files */
    if (close(in_file) == -1 || close(out_file) == -1)
    {
        error_mes("Error closing files", "");
    }

    return true;
}
