/* 
 * cui.cpp
 *
 * Copyright (c) 2004-2006,2008,2010,2011 Arnout Engelen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


/* NetHogs console UI */
#include <string>
#include <pwd.h>
#include <sys/types.h>
#include <cstdlib>
#include <cerrno>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <ncurses.h>
#include "nethogs.h"
#include "process.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <sys/socket.h>

//added by wnh for recording timestamp
#include <time.h>

std::string * caption;
extern const char version[];
extern ProcList * processes;
extern timeval curtime;

extern Process * unknowntcp;
extern Process * unknownudp;
extern Process * unknownip;

extern bool sortRecv;

extern int viewMode;

extern unsigned refreshlimit;
extern unsigned refreshcount;

extern bool pageDown;
extern bool pageUp;

#define PID_MAX 4194303

//std::map< std::string,int > m_trans;

class Line
{
public:
	Line (const char * name, unsigned long inode, double n_recv_value, double n_sent_value, pid_t pid, uid_t uid, const char * n_devicename, std::map<std::string,int> imap)
	{
		assert (pid >= 0);
		assert (pid <= PID_MAX);
		m_name = name;
		sent_value = n_sent_value;
		recv_value = n_recv_value;
		devicename = n_devicename;
		m_pid = pid;
		m_uid = uid;
		m_inode = inode;
		assert (m_pid >= 0);
        m_trans = imap;
	}

	void show (int row, unsigned int proglen);
	void log ();

	double sent_value;
	double recv_value;
private:
    //std::map< std::string,int> m_tran ;
	const char * m_name;
	const char * devicename;
	pid_t m_pid;
	uid_t m_uid;
	unsigned long m_inode;
    std::map<std::string,int> m_trans;
};

#include <sstream>

std::string itoa(int i)
{
	std::stringstream out;
	out << i;
	return out.str();
}

char* printSystemTime(){
    time_t now;
    time(&now);
    struct tm *timenow;
    timenow = localtime(&now);
    char* dest = (char*)calloc(30,sizeof(char*));
    char* src = asctime(timenow);
    int i = 0;
    while(src[i] != '\n'){
        dest[i] = src[i];
        i++;
    }
    return dest;
}

/**
 * @returns the username that corresponds to this uid 
 */
std::string uid2username (uid_t uid)
{
	struct passwd * pwd = NULL;
	errno = 0;

	/* points to a static memory area, should not be freed */
	pwd = getpwuid(uid);

	if (pwd == NULL)
		if (errno == 0)
			return itoa(uid);
		else
			forceExit(false, "Error calling getpwuid(3) for uid %d: %d %s", uid, errno, strerror(errno));
	else
		return std::string(pwd->pw_name);
}

//change the out put to cout in order to make the result visible , nianhui@2016/07/13
void Line::show (int row, unsigned int proglen)
{
    std::ofstream outfile;
    outfile.open("nethogs.log",std::ios::app);
	assert (m_pid >= 0);
	assert (m_pid <= PID_MAX);

	/*if (m_pid == 0)
		mvprintw (row, 6, "?");
	else
		mvprintw (row, 0, "%7d", m_pid);*/
	std::string username = uid2username(m_uid);
	//mvprintw (row, 8, "%s", username.c_str());
	/*if (strlen (m_name) > proglen) {
		// truncate oversized names
		char * tmp = strdup(m_name);
		char * start = tmp + strlen (m_name) - proglen;
		start[0] = '.';
		start[1] = '.';
		mvprintw (row, 8 + 9, "%s", start);
		free (tmp);
	} else {
		mvprintw (row, 8 + 9, "%s", m_name);
	}*/
	//mvprintw (row, 8 + 9 + proglen + 2, "%s", devicename);
	//mvprintw (row, 8 + 9 + proglen + 2 + 6, "%10.3f", sent_value);
	//mvprintw (row, 8 + 9 + proglen + 2 + 6 + 9 + 3, "%10.3f", recv_value);
	if (viewMode == VIEWMODE_KBPS)
	{
		//mvprintw (row, 8 + 9 + proglen + 2 + 6 + 9 + 3 + 11, "KB/sec");
        char* ltime = printSystemTime();
        std::cout << "["<< ltime <<"]:"<< m_pid << "\t" << m_inode << "\t" << username.c_str() << "\t"<< m_name << "\t" << devicename << "\t" << sent_value << "\t" << recv_value << "\t" << "KB/sec@wnh" << std::endl;
        //outfile << "["<< ltime <<"]:"<< m_pid << "\t" << m_inode << "\t" << username.c_str() << "\t"<< m_name << "\t" << devicename << "\t" << sent_value << "\t" << recv_value << "\t" <<     "KB/sec@wnh" << std::endl;
	    free(ltime);
    }

	else if (viewMode == VIEWMODE_TOTAL_MB)
	{
		//mvprintw (row, 8 + 9 + proglen + 2 + 6 + 9 + 3 + 11, "MB    ");
        std::cout << m_pid << "\t" << m_name << "\t" << devicename << "\t" << sent_value << "\t" << recv_value << "\t" << "MB  " << std::endl; 
	}
	else if (viewMode == VIEWMODE_TOTAL_KB)
	{
		//mvprintw (row, 8 + 9 + proglen + 2 + 6 + 9 + 3 + 11, "KB    ");
        char * ltime = printSystemTime();
        //added by wnh@201608/29
        //iterate the map structure 
        std::map<std::string,int>::iterator iter = m_trans.begin();
        std::string result = "";
        for( ;iter != m_trans.end(); iter++ ){
            //char tmp[15];

            result += ( iter->first + ":" + itoa(iter->second) + ";" );
        }
        m_trans.clear();
        //std::cout << m_pid << "\t" << m_name << "\t" << devicename << "\t" << sent_value << "\t" << recv_value << "\t" << "KB  " << std::endl;
        std::cout << "["<< ltime <<"]:"<< m_pid << "\t" << m_inode << "\t" << username.c_str() << "\t"<< m_name << "\t" << devicename << "\t" << sent_value << "\t" << recv_value << "\t" <<     "KB/sec@wnh" << std::endl;
        outfile <<  "["<< ltime <<"]:"<< m_pid << "\t" << m_inode << "\t" << username.c_str() << "\t"<< m_name << "\t" << devicename << "\t" << sent_value << "\t" << recv_value << "\t" <<         "KB/sec@wnh" << " " << result <<std::endl;
        free(ltime);
	}

	else if (viewMode == VIEWMODE_TOTAL_B)
	{
		//mvprintw (row, 8 + 9 + proglen + 2 + 6 + 9 + 3 + 11, "B     ");
        std::cout << m_pid << "\t" << m_name << "\t" << devicename << "\t" << sent_value << "\t" << recv_value << "\t" << "B  " << std::endl;
	}
    outfile.close();
}

void Line::log() {
	std::cout << m_inode << "\t" << m_name << '/' << m_pid << '/' << m_uid << "\t" << sent_value << "\t" << recv_value << std::endl;
}

int GreatestFirst (const void * ma, const void * mb)
{
	Line ** pa = (Line **)ma;
	Line ** pb = (Line **)mb;
	Line * a = *pa;
	Line * b = *pb;
	double aValue;
	if (sortRecv)
	{
		aValue = a->recv_value;
	}
	else
	{
		aValue = a->sent_value;
	}

	double bValue;
	if (sortRecv)
	{
		bValue = b->recv_value;
	}
	else
	{
		bValue = b->sent_value;
	}

	if (aValue > bValue)
	{
		return -1;
	}
	if (aValue == bValue)
	{
		return 0;
	}
	return 1;
}

void init_ui ()
{
	WINDOW * screen = initscr();
	raw();
	noecho();
	cbreak();
	nodelay(screen, TRUE);
	caption = new std::string ("NetHogs");
	caption->append(getVersion());
    char* ltime = printSystemTime();
    caption->append(ltime);
    free(ltime);
	//caption->append(", running at ");
}

void exit_ui ()
{
	clear();
	endwin();
	delete caption;
}

void ui_tick ()
{
	switch (getch()) {
		case 'q':
			/* quit */
			quit_cb(0);
			break;
		case 's':
			/* sort on 'sent' */
			sortRecv = false;
			break;
		case 'r':
			/* sort on 'received' */
			sortRecv = true;
			break;
		case 'm':
			/* switch mode: total vs kb/s */
			viewMode = (viewMode + 1) % VIEWMODE_COUNT;
			break;
		case 'u':
			pageUp = true;
			break;
		case 'd':
			pageDown = true;
			break;
	}
}

float tomb (u_int32_t bytes)
{
	return ((double)bytes) / 1024 / 1024;
}
float tokb (u_int32_t bytes)
{
	return ((double)bytes) / 1024;
}
float tokbps (u_int32_t bytes)
{
	return (((double)bytes) / PERIOD) / 1024;
}

/** Get the kb/s values for this process */
void getkbps (Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;

	/* walk though all this process's connections, and sum
	 * them up */
	ConnList * curconn = curproc->connections;
	ConnList * previous = NULL;
	while (curconn != NULL)
	{
		if (curconn->getVal()->getLastPacket() <= curtime.tv_sec - CONNTIMEOUT)
		{
			/* stalled connection, remove. */
			ConnList * todelete = curconn;
			Connection * conn_todelete = curconn->getVal();
			curconn = curconn->getNext();
			if (todelete == curproc->connections)
				curproc->connections = curconn;
			if (previous != NULL)
				previous->setNext(curconn);
			delete (todelete);
			delete (conn_todelete);
		}
		else
		{
			u_int32_t sent = 0, recv = 0;
			curconn->getVal()->sumanddel(curtime, &recv, &sent);
			sum_sent += sent;
			sum_recv += recv;
			previous = curconn;
			curconn = curconn->getNext();
		}
	}
	*recvd = tokbps(sum_recv);
	*sent = tokbps(sum_sent);
}

/** get total values for this process */
std::map<std::string,int> gettotal(Process * curproc, u_int32_t * recvd, u_int32_t * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
	ConnList * curconn = curproc->connections;
    std::map<std::string,int> m_trans;
	while (curconn != NULL)
	{
		Connection * conn = curconn->getVal();
		sum_sent += conn->sumSent;
		sum_recv += conn->sumRecv;
        //add by wnh@2016/08/29
        //the purpose is to construct the m_trans structure for later use;
        //
        std::string sourceip(inet_ntoa(conn->refpacket->sip));
        std::string destip(inet_ntoa(conn->refpacket->dip));
        std::string key = sourceip + "->" + destip;
        
        //std::map<std::string,int> m_trans;
        m_trans[key] += ( conn->sumSent + conn->sumRecv );

		curconn = curconn->getNext();
	}
	//std::cout << "Sum sent: " << sum_sent << std::endl;
	//std::cout << "Sum recv: " << sum_recv << std::endl;
	*recvd = sum_recv;
	*sent = sum_sent;
    return m_trans;
}

void gettotalmb(Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
	gettotal(curproc, &sum_recv, &sum_sent);
	*recvd = tomb(sum_recv);
	*sent = tomb(sum_sent);
}

/** get total values for this process */
std::map<std::string,int> gettotalkb(Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
    std::map<std::string,int> m_trans;
	m_trans=gettotal(curproc, &sum_recv, &sum_sent);
	*recvd = tokb(sum_recv);
	*sent = tokb(sum_sent);
    std::map<std::string,int>::iterator iter = m_trans.begin();
    for(;iter != m_trans.end();iter++){
        iter->second = tokb(iter->second);
    }
    return m_trans;
}

void gettotalb(Process * curproc, float * recvd, float * sent)
{
	u_int32_t sum_sent = 0,
	  	sum_recv = 0;
	gettotal(curproc, &sum_recv, &sum_sent);
	//std::cout << "Total sent: " << sum_sent << std::endl;
	*sent = sum_sent;
	*recvd = sum_recv;
}

void show_trace(Line * lines[], int nproc) {
	std::cout << "\nRefreshing:\n";

	/* print them */
	for (int i=0; i<nproc; i++)
	{
		lines[i]->log();
		delete lines[i];
	}

	/* print the 'unknown' connections, for debugging */
	ConnList * curr_unknownconn = unknowntcp->connections;
	while (curr_unknownconn != NULL) {
		std::cout << "Unknown connection: " <<
			curr_unknownconn->getVal()->refpacket->gethashstring() << std::endl;

		curr_unknownconn = curr_unknownconn->getNext();
	}
}

void show_ncurses(Line * lines[], int nproc) {
	int rows; // number of terminal rows
	int cols; // number of terminal columns
	unsigned int proglen; // max length of the "PROGRAM" column
	static unsigned int current_page = 0;

	double sent_global = 0;
	double recv_global = 0;

    std::ofstream outfile;
    outfile.open("nethogs.log",std::ios::app);

	getmaxyx(stdscr, rows, cols);	 /* find the boundaries of the screeen */
	scrollok(stdscr, TRUE);

	if (cols < 62) {
		clear();
		mvprintw(0,0, "The terminal is too narrow! Please make it wider.\nI'll wait...");
		return;
	}

	if (cols > PROGNAME_WIDTH) cols = PROGNAME_WIDTH;

	proglen = cols - 55;

	clear();
	//mvprintw (0, 0, "%s", caption->c_str());
    std::cout << caption->c_str() << "\n" << std::endl;
    outfile << caption->c_str() << std::endl;
	attron(A_REVERSE);
	//mvprintw (2, 0, "    PID USER     %-*.*s  DEV        SENT      RECEIVED       ", proglen, proglen, "PROGRAM");
    std::cout << "PID" << "\t" << "INODE" << "\t" << "USER" << "\t" << "DEV" << "\t" << "SENT" << "\t" << "RECEIVED" << "\t" << "PROGRAM" << std::endl;
    outfile << "PID" << "\t" << "INODE" << "\t" << "USER" << "\t" << "DEV" << "\t" << "SENT" << "\t" << "RECEIVED" << "\t" << "PROGRAM" << std::endl;
    attroff(A_REVERSE);

	/* print them */
	int i;
	for (i=0; i<nproc; i++)
	{
//		if (i+3 < rows)
		lines[i]->show(i+3, proglen);
		recv_global += lines[i]->recv_value;
		sent_global += lines[i]->sent_value;
		delete lines[i];
	}
	if (pageDown) {
		current_page++;
		pageDown = false;	
	} else if (pageUp) {
		current_page--;
		pageUp = false;
	}
	scrl(current_page * (rows - 3));

	attron(A_REVERSE);
	int totalrow = std::min(rows-1, 3+1+i);
	//mvprintw (totalrow, 0, "  TOTAL        %-*.*s          %10.3f  %10.3f ", proglen, proglen, " ", sent_global, recv_global);
    //std::cout << "TOTAL" << setprecision(3) <<sent_global << "\t" << setprecision(3) << recv_global << std::endl;
	if (viewMode == VIEWMODE_KBPS)
	{
		//mvprintw (3+1+i, cols - 7, "KB/sec ");
         std::cout << "TOTAL " << std::setprecision(3) <<sent_global << "\t" << std::setprecision(3) << recv_global << "\t"<< "KB/sec@wnh" << std::endl;
         outfile << "TOTAL " << std::setprecision(3) <<sent_global << "\t" << std::setprecision(3) << recv_global << "\t"<< "KB/sec@wnh" << std::endl;
	} else if (viewMode == VIEWMODE_TOTAL_B) {
		mvprintw (3+1+i, cols - 7, "B      ");
	} else if (viewMode == VIEWMODE_TOTAL_KB) {
		mvprintw (3+1+i, cols - 7, "KB     ");
	} else if (viewMode == VIEWMODE_TOTAL_MB) {
		mvprintw (3+1+i, cols - 7, "MB     ");
	}
	attroff(A_REVERSE);
	//mvprintw (totalrow+1, 0, "");
    std::cout << " "<< std::endl;
    outfile << " " << std::endl;
    outfile.close();

    //read /proc/net/tcp file

    /*std::string buff;
    std::ifstream infile;
    std::cout << "print out /proc/net/tcp" << std::endl;

    infile.open("/proc/net/tcp");
    if(!infile){
        std::cout << "open file error" << std::endl;
    }

    while(getline(infile,buff)){
        std::cout<< buff << std::endl;
    }
    infile.close();*/

    refresh();
}

// Display all processes and relevant network traffic using show function
void do_refresh()
{
	refreshconninode();
	refreshcount++;

	ProcList * curproc = processes;
	ProcList * previousproc = NULL;
	int nproc = processes->size();
	/* initialise to null pointers */
	Line * lines [nproc];
	int n = 0;
    std::map<std::string,int> m_trans;

#ifndef NDEBUG
	// initialise to null pointers
	for (int i = 0; i < nproc; i++)
		lines[i] = NULL;
#endif

	while (curproc != NULL)
	{
		// walk though its connections, summing up their data, and
		// throwing away connections that haven't received a package
		// in the last PROCESSTIMEOUT seconds.
		assert (curproc != NULL);
		assert (curproc->getVal() != NULL);
		assert (nproc == processes->size());

		/* remove timed-out processes (unless it's one of the the unknown process) */
		if ((curproc->getVal()->getLastPacket() + PROCESSTIMEOUT <= curtime.tv_sec)
				&& (curproc->getVal() != unknowntcp)
				&& (curproc->getVal() != unknownudp)
				&& (curproc->getVal() != unknownip))
		{
			if (DEBUG)
				std::cout << "PROC: Deleting process\n";
			ProcList * todelete = curproc;
			Process * p_todelete = curproc->getVal();
			if (previousproc)
			{
				previousproc->next = curproc->next;
				curproc = curproc->next;
			} else {
				processes = curproc->getNext();
				curproc = processes;
			}
			delete todelete;
			delete p_todelete;
			nproc--;
			//continue;
		}
		else
		{
			// add a non-timed-out process to the list of stuff to show
			float value_sent = 0,
				value_recv = 0;

			if (viewMode == VIEWMODE_KBPS)
			{
				//std::cout << "kbps viemode" << std::endl;
				getkbps (curproc->getVal(), &value_recv, &value_sent);
			}
			else if (viewMode == VIEWMODE_TOTAL_KB)
			{
				//std::cout << "total viemode" << std::endl;
				m_trans = gettotalkb(curproc->getVal(), &value_recv, &value_sent);
			}
			else if (viewMode == VIEWMODE_TOTAL_MB)
			{
				//std::cout << "total viemode" << std::endl;
				gettotalmb(curproc->getVal(), &value_recv, &value_sent);
			}
			else if (viewMode == VIEWMODE_TOTAL_B)
			{
				//std::cout << "total viemode" << std::endl;
				gettotalb(curproc->getVal(), &value_recv, &value_sent);
			}
			else
			{
				forceExit(false, "Invalid viewMode: %d", viewMode);
			}
			uid_t uid = curproc->getVal()->getUid();
#ifndef NDEBUG
			//struct passwd * pwuid = getpwuid(uid);
			//assert (pwuid != NULL);
			// value returned by pwuid should not be freed, according to
			// Petr Uzel.
			//free (pwuid);
#endif
			assert (curproc->getVal()->pid >= 0);
			assert (n < nproc);

			lines[n] = new Line (curproc->getVal()->name, curproc->getVal()->getInode(), value_recv, value_sent,
					curproc->getVal()->pid, uid, curproc->getVal()->devicename, m_trans);
			previousproc = curproc;
			curproc = curproc->next;
			n++;
#ifndef NDEBUG
			assert (nproc == processes->size());
			if (curproc == NULL)
				assert (n-1 < nproc);
			else
				assert (n < nproc);
#endif
		}
	}

	/* sort the accumulated lines */
	qsort (lines, nproc, sizeof(Line *), GreatestFirst);

	if (tracemode || DEBUG)
		show_trace(lines, nproc);
	else
		show_ncurses(lines, nproc);

	if (refreshlimit != 0 && refreshcount >= refreshlimit)
		quit_cb(0);
}
