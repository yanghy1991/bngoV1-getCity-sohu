#include "city_sohu.h"
#include "cacertinmem.h"
#include "bngo_aes.h"
#include "bngo_info.h"
#include "URLEncode.h"

/**
*功能:对http响应的json字符串进行解析,解析出参数city
*参数:
*json_str:json字符串
*city:用来存储city
*
*/
int sohu_city_parse_json(char *json_str,char *city_name,char *city_code)
{
	//printf("json_str = %s\n",json_str);
	cJSON * root = NULL;
    cJSON * item = NULL;
	root = cJSON_Parse(json_str);
	if (!root) 
    {
		printf("%s : create json object return.\n",__func__);
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
    }
	else
	{
		item = cJSON_GetObjectItem(root, city_name);
		if(item ==NULL)
		{
			printf("there is no the item %s\n",city_name);
			return -1;
		}
		char *tmp = cJSON_Print(item);
		tmp[strlen(tmp)-1] = 0;
		strcpy(city_code,tmp+1);
		free(tmp);
		return 0;
	}
}

int get_file_line(char *pInputName, char *pOutputBuf, int cnt)  
{  
    FILE * fp;  
    int i=0;  
    char * line = NULL;  
    size_t len = 0;  
    ssize_t read;  
  
    fp = fopen(pInputName, "r");  
    if (fp == NULL)
	{
		printf("open city_json_use.txt error.\n");
		return -1;
	}		
    if(cnt < 0)
	{
		printf("line number must >= 0.\n");
		return -1;
	}		
           
           
    while ((read = getline(&line, &len, fp)) != -1) {  
        ++i;  
        if(i >= cnt)  
            break;  
    }  
  
    if (line)  
    {  
        memcpy(pOutputBuf,line,strlen(line));  
        free(line);  
        return 0;   
    }  
	printf("get line error.\n");
    return -1;  
}

int bkup_file(char *src_path,char *dest_path)
{
	char city_buf[SOHU_COM_BUFSIZE];
	memset(city_buf,0,SOHU_COM_BUFSIZE);
	//检测文件是否存在
	if (access(src_path,F_OK) == -1)
	{
		printf("%s not exist.\n",src_path);
		return -1;
	}
	else
	{
		//从文件中读取当前城市名
		int fd = -1;
		int ret = 0;
		//open file
		fd = open(src_path, O_RDONLY);
		if (-1 == fd)
		{
			printf("open %s error.\n",src_path);
			return -1;
		}
		//read file
		ret = read(fd, city_buf, SOHU_COM_BUFSIZE-1);
		if(ret < 0)
		{
			printf("read city name error.\n");
			return -1;
		}
		else
		{
			if((strlen(city_buf) <= 0) || (strstr(city_buf,"nodata") != NULL))
			{
				printf("%s data error.\n",src_path);
				return -1;
			}
			else
			{
				printf("city name is %s\n",city_buf);
				char command[SOHU_COM_BUFSIZE];
				memset(command,0,SOHU_COM_BUFSIZE);
				strcat(command,"cp ");
				strcat(command,src_path);
				strcat(command," ");
				strcat(command,dest_path);
				system(command);
			}
		}			
	}
	
	return 0;
}
//get wifi_mac
void get_mac(char *Str)
{
	char *dest = Str;
	while(*Str != '\0')
	{
		if(*Str != ':' && *Str != 0x0a)
		{
			*dest++ = *Str;
		}
		
		++Str;
	}
	*dest = '\0';
	//printf("wifi_mac:%s \n",dest);
	
}

int citi_json_result(char *city,char *json_str)
{
	cJSON * root = NULL;
	cJSON * code = NULL;
	cJSON * data = NULL;

	
	root = cJSON_Parse(json_str);
	if (!root) 
    {
		printf("%s : parse json error.\n",__func__);  
		return -1;
    }	

	//获取 error 错误数据，返回原因
	code = cJSON_GetObjectItem(root, "code");
	if(code == NULL)
	{
		cJSON_Delete(root);
		return -1;//获取err_no失败
	} 

	//获取 error 错误数据，返回原因
	data = cJSON_GetObjectItem(root, "data");
	if(data == NULL)
	{
		cJSON_Delete(root);
		return -1;//获取err_no失败
	}

	if(code->valueint == 2000){
		memcpy(city,data->valuestring,strlen(data->valuestring)+1);
	} else if(code->valueint == 2001){
		strcpy(city,"2001");
	} else {
		cJSON_Delete(root);
		return -1;//获取err_no失败
	}

	cJSON_Delete(root);
	return 0;
}



int main()
{
	/*
	//备份城市文件
	if(-1 == bkup_file("data/sohu_city_gb.txt","data/sohu_city_gb_bk.txt"))
		printf("backup file error.\n");
	if(-1 == bkup_file("data/sohu_city_code.txt","data/sohu_city_code_bk.txt"))
		printf("backup file error.\n");
	if(-1 == bkup_file("data/sohu_city.txt","data/sohu_city_bk.txt"))
		printf("backup file error.\n");
	*/
	int fd = -1;
	int ret = 0;
	
	//从网络获取当前城市名
	char *url = "pv.sohu.com/cityjson?ie=utf-8";
	char city[SOHU_COM_BUFSIZE];
	memset(city,0,SOHU_COM_BUFSIZE);	
	if(-1 == sohu_get_city(url,city))
	{
		printf("city http request error.\n");
		strcpy(city,"CHINA");
		//goto end;
	}
	
	//printf("city = %s\n",city);
	
	if(strlen(city) <= 0)
	{
		printf("get city error.\n");
		strcpy(city,"CHINA");
	}
	
	
	
	char city_code[SOHU_SMALL_BUFSIZE];
	memset(city_code,0,SOHU_SMALL_BUFSIZE);

	char city_urlencode[SOHU_COM_BUFSIZE] = {0};
	//url 加密city
	URLEncode(city, strlen(city), city_urlencode, SOHU_COM_BUFSIZE);

	/*
	//检测文件是否存在和获取城市代码
	if (access("data/city_json_use.txt",F_OK) == -1)
	{
		printf("data/city_json_use.txt not exist.\n");
		goto end;
	}
	else
	{
		int i = 0;
		for(i=1;i<36;i++)
		{
			char line_buf[SOHU_LINE_BUFSIZE];
			memset(line_buf,0,SOHU_LINE_BUFSIZE);
			if(-1 == get_file_line("data/city_json_use.txt",line_buf,i))
			{
				printf("get line from file error.\n");
				goto end;
			}
			//printf("line buf = %s\n",line_buf);
			if(-1 == sohu_city_parse_json(line_buf,city,city_code))
			{
				printf("get city_code error.\n");				
			}
			else
			{
				if(strlen(city_code) != 0)
				{
					printf("city_code = %s\n",city_code);
					break;
				}	
			}	
		}
	}
	
	//存储天气预报城市代码
	if(strlen(city_code) <= 0)
	{
		printf("get city_code error.\n");
		strcpy(city_code,"nodata");
	}
	
	//DIR *dirp = NULL;
	if(opendir("data") == NULL)  
	{  
		if(mkdir("data",0777) == -1)
		{
			printf("mkdir -data- error.\n");
			goto end;
		}
	}
			
	
	//open file
	fd = open("data/sohu_city_code.txt", O_RDWR|O_CREAT|O_TRUNC);
	if (-1 == fd)
	{
		printf("open sohu_city_code.txt error.\n");
		goto end;
	}
	//write file
	ret = write(fd, city_code, strlen(city_code));
	if(-1 == ret)
	{
		printf("write sohu_city_code.txt error.\n");
		goto end;
	}
	close(fd);	
	
	//把城市名的gb2312码存入文件
	char city_gb[SOHU_COM_BUFSIZE];
	memset(city_gb,0,SOHU_COM_BUFSIZE);
	//检测文件是否存在和获取城市代码
	if (access("data/city_gb_json_use.txt",F_OK) == -1)
	{
		printf("data/city_gb_json_use.txt not exist.\n");
		goto end;
	}
	else
	{
		int i = 0;
		for(i=1;i<36;i++)
		{
			char line_buf[SOHU_LINE_BUFSIZE];
			memset(line_buf,0,SOHU_LINE_BUFSIZE);
			if(-1 == get_file_line("data/city_gb_json_use.txt",line_buf,i))
			{
				printf("get line from file error.\n");
				goto end;
			}
			//printf("line buf = %s\n",line_buf);
			if(-1 == sohu_city_parse_json(line_buf,city,city_gb))
			{
				printf("get city_gb error.\n");				
			}
			else
			{
				if(strlen(city_gb) != 0)
				{
					printf("city_gb == %s\n",city_gb);
					break;
				}	
			}	
		}
	}
	
	//存储城市名国标码
	if(strlen(city_gb) <= 0)
	{
		printf("get city_code error.\n");
		strcpy(city_gb,"nodata");
	}
			
	//open file
	fd = open("data/sohu_city_gb.txt", O_RDWR|O_CREAT|O_TRUNC);
	if (-1 == fd)
	{
		printf("open sohu_city_gb.txt error.\n");
		goto end;
	}
	//write file
	ret = write(fd, city_gb, strlen(city_gb));
	if(-1 == ret)
	{
		printf("write sohu_city_gb.txt error.\n");
		goto end;
	}
	close(fd);
	*/
	//存储城市名汉字
	//open file
	
	/*
	 *  get wlan mac
	 */
	 //{"BINGO_ID":"SZH1-BINGOV1-ac83f3ca8fea","location":"深圳"}
	 char deviceID[256] = "{\"BINGO_ID\":\"";
	 FILE *devFd;

	 /*
	 int w_fd;
	 FILE *devFd;
	 char buf[25] = {0};
	 char bngoID[30] = "SZH1-BINGOV1-";
	 
	 

	 
	
	w_fd = open("/sys/class/net/wlan0/address",O_RDONLY);

	if(w_fd < 0){
		printf("open mac file error!!\n");
		return -1;
	}
	
	read(w_fd,buf,25);
	close(w_fd);
	
	get_mac(buf);
	strcat(bngoID,buf);
	*/
	char bngo_aes[100] = {0};

	if(get_bngoID(bngo_aes) != 0){
		printf("[%s] [%d] get bngoID failed \n",__func__,__LINE__);
		return -1;
	}


	
	//设备注册
	char strResponse[5120] = {0};
	char strGet[256] = {0};

	//测试服务器
	//strcat(strGet,"http://47.107.25.1:80/irelf/v1/device/register?bngoid=");
	//strcat(strGet,bngo_aes);
	//strcat(strGet,"&city=");
	//strcat(strGet,city_urlencode);
	
	//printf("[%s] strGet:%s\n",__func__,strGet);
	//http_get(strGet,strResponse);
	
	//printf("设备注册成功 [%s]\n",strResponse);

	register_bngo(bngo_aes,city_urlencode,strResponse);

	//解析是否正常注册  2001 注册失败，请使用手机注册  ；2000 注册成功
	char *json_src = NULL;
	json_src = strstr(strResponse,"{");
	
	if(json_src == NULL){
		strcpy(city,"2001");
	}

	int err = citi_json_result(city,strResponse);
	if(err != 0){
		strcpy(city,"2001");
	}

	/////////////////////////////////////////////////////////////
	strcat(deviceID,bngo_aes);
	strcat(deviceID,"\",\"location\":\"");
	strcat(deviceID,city);
	strcat(deviceID,"\",\"netconfig\":\"NO\",\"NAME\":\"");
	
	char name[128] = {0};
	if (access("/usr/data/DEVICENAME",F_OK) != 0){
		strcpy(name,"BNGO");
	} else {
		/*************记录历史命令*******************/
		devFd = fopen("/usr/data/DEVICENAME","r+");
		if(devFd == NULL){
			printf("%s: open record error!!!!.\n",__func__);
			return -1;
		}
		fread(name,128,1,devFd);
		fclose(devFd);	
	}
	
	strcat(deviceID,name);
	strcat(deviceID,"\"}");
	//strcat(deviceID,"\"}");
	
	fd = open("/usr/data/BINGO_MSG.bngo", O_RDWR|O_CREAT|O_TRUNC | O_SYNC);
	if(fd < 0){
		printf("create deviceID.txt file error!!\n");
		return -2;
	}
	write(fd, deviceID, strlen(deviceID));
	close(fd);


	/*
	fd = open("data/sohu_city.txt", O_RDWR|O_CREAT|O_TRUNC);
	if (-1 == fd)
	{
		printf("open sohu_city.txt error.\n");
		goto end;
	}
	//write file
	ret = write(fd, city, strlen(city));
	if(-1 == ret)
	{
		printf("write sohu_city.txt error.\n");
		goto end;
	}
	close(fd);
	*/
	
	//end:
	return 0;
}
