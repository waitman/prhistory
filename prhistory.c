#define CURL_STATICLIB
#include <stdio.h>
#include <sqlite3.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_PATH 10

size_t 
write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) 
{
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}
int 
progress_func(void* ptr, double TotalToDownload, double NowDownloaded, 
                    double TotalToUpload, double NowUploaded)
{
fprintf(stdout,".");
fflush(stdout);
return (0);
}

int
clear_progress(void* ptr, double TotalToDownload, double NowDownloaded,
                    double TotalToUpload, double NowUploaded)
{
return (0);
}



int
dodld(void)
{
CURL *curl;
FILE *fp;
CURLcode res;
const char *url = "http://www.burplex.com/px/ports-pr.db";
char outfilename[FILENAME_MAX] = "/var/db/ports/ports-pr.db";
curl = curl_easy_init();
if (curl) {
	fprintf(stdout,"Downloading ports-pr.db to /var/db/ports/...");
	fflush(stdout);

	fp = fopen(outfilename,"w");
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	fclose(fp);
	fprintf(stdout,".complete\n\n");
}
return (1);
}

int
doprnt(char *pr)
{
CURL *curl;
FILE *fp;
CURLcode res;
char url[256] = {0};
char outfilename[FILENAME_MAX] = "/dev/tty";
sprintf(url,"http://www.freebsd.org/cgi/query-pr.cgi?pr=ports%%2F%s&f=raw",pr);
curl = curl_easy_init();
if (curl) {
        fprintf(stdout,"Attempting to fetch PR from WWW please wait a moment...\n\n");
        fflush(stdout);

        fp = fopen(outfilename,"w");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, clear_progress);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
	
}
return (1);
}

static int 
callback(void *NotUsed, int argc, char **argv, char **azColName)
{
int i;
for(i=0; i<argc; i++){
	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
}
printf("\n");
return 0;
}

int 
main(int argc, char **argv)
{
int extras = 0;
int legend = 0;
int lookup = 0;
char cwd[1024] = {0};
char sql[256] = {0};
sqlite3 *db;
char *zErrMsg = 0;
int rc;
int count=0;
struct stat st;
size_t i;
char *token,*path,*cp[MAX_PATH];
while ((argc>1) && (argv[1][0]=='-')) {
	if (argv[1][1]=='c') {
		extras=1; // switch c = show closed pr TOO

/*
NOTE: GNATS search tool is not currently showing 
closed PR for some reason. This switch will not return 
the extra PR (closed) .
*/

	}
	if (argv[1][1]=='u') {
		dodld();
		return(0);
	}
	if (argv[1][1]=='l') {
		legend=1;
	}
	if (argv[1][1]=='h') {
		fprintf(stdout,"prhistory: usage\n\nThis program searches the GNATS database for PR corresponding to a port.\n\nCommand Switches:\n-h I will show HELP\n-l I will show legend\n-u We will update PR db from the WWW\n-c You will see closed PR TOO (AS WELL) if AVAILABLE\n-p Display PR (ports only).\n\nRun without search term and we will look for port in cwd.\n\nExample:\n# cd /usr/ports/lang/perl5.16\n# prhistory\n\nExample:\n# prhistory perl\n\nExample: lookup PR\n# prhistory -p 123456\n\ndb file is stored in /var/db/ports/ports-pr.db (SQLite3)\n\n");
		return(0);
	}
	if (argv[1][1]=='p') {
		lookup=1;
	}
	++argv;
	--argc;
}

if (argc>1) {

	if (lookup) {
		doprnt(argv[1]);
		return(0);
	}

	fprintf(stdout,"Search Description: %s\n",argv[1]);
	if (extras) {
		sprintf(sql,"SELECT * FROM pr WHERE desc LIKE '%%%s%%' ORDER BY postdate ASC",argv[1]);
	} else {
		sprintf(sql,"SELECT * FROM pr WHERE desc LIKE '%%%s%%' AND status!='c' ORDER BY postdate ASC",argv[1]);
	}

	
} else {

	if (getcwd(cwd,sizeof(cwd))!=NULL) {
		fprintf(stdout,"Current Path: %s\n",cwd);
		path = strdup(cwd);
		while ((token = strsep(&path, "/")) != NULL)
			if (token && count<MAX_PATH) cp[count++]=token;
	} else {
		fprintf(stderr,"getcwd() error");
	}

	fprintf(stdout,"Category: %s\n",cp[count-2]);
	fprintf(stdout,"Port: %s\n",cp[count-1]);
	if (extras) {
		sprintf(sql,"SELECT * FROM pr WHERE desc LIKE '%%%s/%s%%' ORDER BY postdate ASC",cp[count-2],cp[count-1]);
	} else {
		sprintf(sql,"SELECT * FROM pr WHERE desc LIKE '%%%s/%s%%' AND status!='c' ORDER BY postdate ASC",cp[count-2],cp[count-1]);
	}

}
fprintf(stdout,"\n");

rc = sqlite3_open("/var/db/ports/ports-pr.db", &db);
if( rc ){
	fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	sqlite3_close(db);
	return(1);
}

rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
if( rc!=SQLITE_OK ){
	fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);
}
sqlite3_close(db);
if (stat("/var/db/ports/ports-pr.db", &st)!=0) {
	fprintf(stderr, "Error getting timestamp of /var/db/ports/ports-pr.db");
} else {
	char outstr[200] = {0};
	struct tm *tmp;
	tmp = localtime(&st.st_mtime);
	strftime(outstr, sizeof(outstr), "%a, %d %b %Y %T %z",tmp);
	printf("Database last updated: %s\n",outstr);
}
if (legend) {
	fprintf(stdout,"Legend:\n");
	fprintf(stdout,"o - open\n");
	fprintf(stdout,"    A problem report has been submitted, no sanity checking performed.\n");
	fprintf(stdout,"a - analyzed\n");
	fprintf(stdout,"    The problem is understood and a solution is being sought.\n");
	fprintf(stdout,"f - feedback\n");
	fprintf(stdout,"    Further work requires additional information from the originator or the community-possibly confirmation of the effectiveness of a proposed solution.\n");
	fprintf(stdout,"p - patched\n");
	fprintf(stdout,"    A patch has been committed, but some issues (MFC and / or confirmation from originator) are still open.\n");
	fprintf(stdout,"s - suspended\n");
	fprintf(stdout,"    The problem is not being worked on, due to lack of information or resources. This is a prime candidate for somebody who is looking for a project to do. If the problem cannot be solved at all, it will be closed, rather than suspended.\n");
	fprintf(stdout,"c - closed\n");
	fprintf(stdout,"    A problem report is closed when any changes have been integrated, documented, and tested-or when fixing the problem is abandoned.\n");
}
return 0;
}
