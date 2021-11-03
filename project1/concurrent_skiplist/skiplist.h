#include <iostream>
#include <sstream>
#include<mutex>
#include<pthread.h>

#define BILLION  1000000000L
#define NPAIRS  88

using namespace std;
 
template<class K,class V,int MAXLEVEL>
class skiplist_node
{
public:
    struct Forward{
	    public:
		    skiplist_node<K,V,MAXLEVEL>* forwards;
		    pthread_mutex_t fwLock;
    };

    Forward fw[MAXLEVEL + 1];
    int ret;

    skiplist_node()
    {
        for ( int i=1; i<=MAXLEVEL; i++ ) {
            fw[i].forwards = NULL;
	    ret = pthread_mutex_init(&fw[i].fwLock, NULL);
        }
	ret = pthread_mutex_init(&rwLock, NULL);
	cnt = 0;
    }
 
    skiplist_node(K searchKey)
    {
        for ( int i=1; i<=MAXLEVEL; i++ ) {
            fw[i].forwards = NULL;
	    ret = pthread_mutex_init(&fw[i].fwLock, NULL);
        }
	ret = pthread_mutex_init(&rwLock, NULL);
	key[0] = searchKey;
	cnt = 1;
    }
    skiplist_node(K searchKey, V val){
 
        for ( int i=1; i<=MAXLEVEL; i++ ) {
            fw[i].forwards = NULL;
	    ret = pthread_mutex_init(&fw[i].fwLock, NULL);
        }
	ret = pthread_mutex_init(&rwLock, NULL);
	key[0] = searchKey;
	value[0] = val;
	cnt = 1;
    }
 
    virtual ~skiplist_node()
    {
    }

    void insert(K k, V v)
    {
	for(int i=0;i<cnt;i++){
	        if( key[i] < k){
		    continue;
		}
		// shift to right
		for(int j=cnt-1;j>=i;j--){
	            key[j+1] = key[j] ;
	            value[j+1] = value[j] ;
		}
		// insert to the right position
	        key[i] = k;
	        value[i] = v;
		cnt++;
		return;
	}
	key[cnt] = k;
	value[cnt] = v;
	cnt++;
	return;
    }

    /*void insert(K k, V v)
    {
            int i = 0;
            while(i<cnt){
                    pthread_mutex_lock(&inLock);
                    if(key[i] < k){
                            pthread_mutex_unlock(&inLock);
                            i++;
                            continue;
                    }
                    pthread_mutex_unlock(&inLock);
                    pthread_mutex_lock(&inLock);
                    if(cnt >= NPAIRS){
                            if(split_flag){
                                pthread_mutex_lock(&cdLock);
                                pthread_cond_wait(&cond, &cdLock);
                                i = cnt + 1;
                                pthread_mutex_unlock(&cdLock);
                            }
                            else{
                                    pthread_mutex_unlock(&inLock);
                                    split_flag = true;
                                    return;
                            }
                    }
                    //shift to right
                    for(int j=cnt-1; j>=i;j--){
                            key[j+1] = key[j];
                            value[j+1] = value[j];
                    }
                    //insert to the right position
                    key[i] = k;
                    value[i] = v;
                    cnt++;
                    pthread_mutex_unlock(&inLock);
                    return;
            }
            pthread_mutex_lock(&inLock);
            if(cnt >= NPAIRS){
                    if(split_flag){
                        pthread_mutex_lock(&cdLock);
                        pthread_cond_wait(&cond, &cdLock);
                        i = cnt + 1;
                        pthread_mutex_unlock(&cdLock);
                    }
                    else{
                            pthread_mutex_unlock(&inLock);
                            split_flag = true;
                            return;
                    }
            }
            key[cnt] = k;
            value[cnt] = v;
            cnt++;
   	    pthread_mutex_unlock(&inLock);
	    return;
	}*/
	
 
    int cnt;
    // change KV to array of structure later
    K key[NPAIRS];   // 4*44   --> 176
    V value[NPAIRS];   // 4*44   --> 176
 // skiplist_node<K,V,MAXLEVEL>* forwards[MAXLEVEL+1];   // 8*17 = 156 bytes --> 128 + 28 bytes
    pthread_mutex_t rwLock;
    // total 352 + 128 + 28 + 4 -->  512 bytes --> 8 cachelines
};
 
///////////////////////////////////////////////////////////////////////////////
 
template<class K, class V, int MAXLEVEL = 16>
class skiplist
{
public:
    typedef K KeyType;
    typedef V ValueType;
    typedef skiplist_node<K,V,MAXLEVEL> NodeType;
 
    skiplist(K minKey,K maxKey):m_pHeader(NULL),m_pTail(NULL),
                                max_curr_level(1),max_level(MAXLEVEL),
                                m_minKey(minKey),m_maxKey(maxKey)
    {
        m_pHeader = new NodeType(m_minKey);
        m_pTail = new NodeType(m_maxKey);
        for ( int i=1; i<=MAXLEVEL; i++ ) {
            m_pHeader->fw[i].forwards = m_pTail;
        }
    }
 
    virtual ~skiplist()
    {
        NodeType* currNode = m_pHeader->fw[1].forwards;
        while ( currNode != m_pTail ) {
            NodeType* tempNode = currNode;
            currNode = currNode->fw[1].forwards;
            delete tempNode;
        }
        delete m_pHeader;
        delete m_pTail;
    }
 
    void insert(K searchKey,V newValue)
    {
        skiplist_node<K,V,MAXLEVEL>* update[MAXLEVEL];
        NodeType* currNode = m_pHeader;
        for(int level=max_curr_level; level >=1; level--) {
	    pthread_mutex_lock(&currNode->fw[level].fwLock);
            while ( currNode->fw[level].forwards->key[0] <= searchKey ) {
		pthread_mutex_unlock(&currNode->fw[level].fwLock);
                currNode = currNode->fw[level].forwards;
		pthread_mutex_lock(&currNode->fw[level].fwLock);
            }
	    ///////////////////lock////////////////////
            update[level] = currNode;
        }
        //currNode = currNode->forwards[1];
	if( currNode->cnt < NPAIRS){
	    currNode->insert(searchKey, newValue);
	}
	else { // split
	    int newlevel = randomLevel();
	    if ( newlevel > max_curr_level ) {
		for ( int level = max_curr_level+1; level <= newlevel; level++ ) {
			pthread_mutex_lock(&m_pHeader->fw[level].fwLock);
			update[level] = m_pHeader;
		}
		max_curr_level = newlevel;
	    }

       	    //currNode = new NodeType(searchKey,newValue);
	    NodeType* newNode = new NodeType();
	    int mid=currNode->cnt/2; 
	    for (int i=mid; i<currNode->cnt; i++){
	        newNode->insert(currNode->key[i], currNode->value[i]);
	    }
	    currNode->cnt = mid;
	    if(newNode->key[0] < searchKey){
	        newNode->insert(searchKey, newValue);
	    }
	    else{
	        currNode->insert(searchKey, newValue);
	    }

	    for ( int lv=1; lv<=newlevel; lv++ ) {
		newNode->fw[lv].forwards = update[lv]->fw[lv].forwards;
		update[lv]->fw[lv].forwards = newNode; // make previous node point to new node
	    }
	}
	for(int level=1; level<=max_curr_level; level++){
		if(update[level] == NULL)
			continue;
		pthread_mutex_unlock(&update[level]->fw[level].fwLock);
	}
    }
 
    void erase(K searchKey)
    {
	    /*
        skiplist_node<K,V,MAXLEVEL>* update[MAXLEVEL];
        NodeType* currNode = m_pHeader;
        for(int level=max_curr_level; level >=1; level--) {
            while ( currNode->forwards[level]->key < searchKey ) {
                currNode = currNode->forwards[level];
            }
            update[level] = currNode;
        }
        currNode = currNode->forwards[1];
        if ( currNode->key == searchKey ) {
            for ( int lv = 1; lv <= max_curr_level; lv++ ) {
                if ( update[lv]->forwards[lv] != currNode ) {
                    break;
                }
                update[lv]->forwards[lv] = currNode->forwards[lv];
            }
            delete currNode;
            // update the max level
            while ( max_curr_level > 1 && m_pHeader->forwards[max_curr_level] == NULL ) {
                max_curr_level--;
            }
        }
	*/
    }
 
    //const NodeType* find(K searchKey)
    V find(K searchKey)
    {
        NodeType* currNode = m_pHeader;
        for(int level=max_curr_level; level >=1; level--) {
            while ( currNode->fw[level].forwards->key[0] <= searchKey ) {
                currNode = currNode->fw[level].forwards;
            }
        }
        // currNode = currNode->forwards[1];
	for(int i=0;i<currNode->cnt;i++){
            if ( currNode->key[i] == searchKey ) {
		//pthread_mutex_unlock(&currNode->rwLock);
                return currNode->value[i];
            }
	}
        //return NULL;
        return -1;
    }
 
    bool empty() const
    {
        return ( m_pHeader->fw[1].forwards == m_pTail );
    }
 
    std::string printList()
    {
	int i=0;
        std::stringstream sstr;
        NodeType* currNode = m_pHeader; //->forwards[1];
        while ( currNode != m_pTail ) {
	    sstr << "(" ;
	    for(int i=0;i<currNode->cnt;i++){
		sstr << currNode->key[i] << "," ;
	    }
	    sstr << ")";
            currNode = currNode->fw[1].forwards;
	    i++;
	    if(i>200) break;
        }
        return sstr.str();
    }
 
    const int max_level;
 
protected:
    double uniformRandom()
    {
        return rand() / double(RAND_MAX);
    }
 
    int randomLevel() {
        int level = 1;
        double p = 0.5;
        while ( uniformRandom() < p && level < MAXLEVEL ) {
            level++;
        }
        return level;
    }
    K m_minKey;
    K m_maxKey;
    mutable mutex m;
    int max_curr_level;
    skiplist_node<K,V,MAXLEVEL>* m_pHeader;
    skiplist_node<K,V,MAXLEVEL>* m_pTail;
};
 
///////////////////////////////////////////////////////////////////////////////
 
