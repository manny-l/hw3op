#include "b5-9.h"
#define TREE_VALUE 9

typedef struct vertex_t* vertex;

struct vertex_t{
	vertex father;
	vertex sons[TREE_VALUE+1];
	int keys[TREE_VALUE];
	int isLeaf;
	int key;
	int numSons;
	char value;
	lock vxLock;
};

struct tree_t{
	vertex root;
	int isEmpty;
	lock treeLock;
};

/*----------------------------------------------------------------
Global parameters
----------------------------------------------------------------*/
//array that holds all the nodes in the search path to be upgraded.
vertex upgradePathArr[50];

/*----------------------------------------------------------------
functions declaration
----------------------------------------------------------------*/
int createVertex(vertex* currNode);
int createLeaf(vertex* currNode,int key,char value);
vertex findForInsert(tree currTree,vertex currNode,int key,int* location, int* found, int flagRoot);
char findNode(vertex currNode,int key, int* found);
int fixTree(tree currTree,vertex currNode);
void releaseSharedPath(tree currTree,vertex tempVertex);
void releaseExclusivePath(tree currTree,vertex tempVertex);
void freeAllSons(vertex currNode);
void upgradePath(tree currTree,vertex lastNode);


/*----------------------------------------------------------------
initTree function:
Output: currTree - the new tree
Return value: 0 - upon success, -1 otherwise
Description: allocation and initialization of a new tree
-----------------------------------------------------------------*/
int initTree(tree* currTree){
	(*currTree) =(tree)malloc(sizeof(struct tree_t));
	if ((*currTree) == NULL){
		printf("Error in memory allocation\n");
		exit(0);
	}
	(*currTree)->root = NULL;
	(*currTree)->isEmpty = 1;
	if (lock_init(&((*currTree)->treeLock))==-1)
	{
		free((*currTree));
		return -1;
	}
	return 0;
}

/*----------------------------------------------------------------
createVertex function:
Output: currNode - the new node
Return value: 0 - upon success, -1 otherwise
Description: creates a new Node that will be an inner node
-----------------------------------------------------------------*/
int createVertex(vertex* currNode){

	int i,j;

	//allocate new node
	if(NULL ==((*currNode)=(vertex)malloc(sizeof(struct vertex_t)))){
		printf("Error in memory allocation\n");
		exit(0);
	}

	//node init
	(*currNode)->father = NULL;
	for (i=0;i<TREE_VALUE-1;i++){
		(*currNode)->sons[i]=NULL;
		(*currNode)->keys[i]=0;
	}
	for(j=i;j<TREE_VALUE+1;j++){
		(*currNode)->sons[j]=NULL;
	}
	(*currNode)->isLeaf = 0;
	(*currNode)->numSons = 0;
	(*currNode)->key = 0;

	//init lock
	if (lock_init(&((*currNode)->vxLock))==-1){
		free(*currNode);
		return -1;
	}

	return 0;
}

/*----------------------------------------------------------------
createLeaf function:
Input:  key - the key of the new leaf
		value - the value of the new leaf
Output: currNode - the new leaf
Return value: 0 - upon success, -1 otherwise
Description: creates a new Node with the given key & value
-----------------------------------------------------------------*/
int createLeaf(vertex* currNode,int key,char value){

	int i;
	//allocate new node
	if(NULL == ((*currNode)=(vertex)malloc(sizeof(struct vertex_t)))){
		printf("Error in memory allocation\n");
		exit(0);
	}

	//leaf initialization
	(*currNode)->father=NULL;
	for (i=0;i<TREE_VALUE-1;i++){
		(*currNode)->sons[i]=NULL;
		(*currNode)->keys[i]=0;
	}
	for(i=i;i<TREE_VALUE+1;i++){
		(*currNode)->sons[i]=NULL;
	}
	(*currNode)->isLeaf=1;
	(*currNode)->numSons=0;
	(*currNode)->key=key;
	(*currNode)->value=value;

	//init lock
	if (lock_init(&((*currNode)->vxLock))==-1){
		free(*currNode);
		return -1;
	}
	return 0;
}

/*----------------------------------------------------------------
findForInsert function: finding nodes by key for the addElm function
Input: currTree - the current tree
	   currNode - our location in this search
	   key - the key we are searching
	   flagRoot - 1 if the currNode is the root, 0 otherwise
Output: found - 1 if we found a node with the given key, 0 - otherwise
Return value: the node where the new leaf should enter
Description: looking for the place to insert the given key.
			 every node in the serch path is locked as MayWrite
-----------------------------------------------------------------*/
vertex findForInsert(tree currTree,vertex currNode,int key,int* location, int* found,int flagRoot){

	int i;
	vertex temp;

	//invalid input
	if ((NULL==location) || (NULL==currNode) || (NULL==found)){
		return NULL;
	}

	//init
	*found = 0;
	if (flagRoot){
		get_may_write_lock(&(currTree->treeLock));
		currNode = currTree->root;
		flagRoot =0;
	}

	//lock the current vortex as MayWrite
	get_may_write_lock(&(currNode->vxLock));

	//compare give key with current vortex keys
	for(i=0;i<(currNode->numSons-1);i++){
		if(key<currNode->keys[i]){
			break;
		}
	}
	//if is not in the first key, means that it is
	//bigger than the current child or equal to it
	if (i){
		i++;
	}
	//if it is the father
	get_read_lock(&(currNode->sons[0]->vxLock));
	if(currNode->sons[0]->isLeaf){
		release_shared_lock(&(currNode->sons[0]->vxLock));
		if ((i>0) && (currNode->sons[i-1]->key==key)){
			*found=1;
			*location=i-1;
			return currNode;
		}
		if(NULL==currNode->sons[i]){
			*location=i;
			return currNode;
		}
		//check if it has the same son already
		if (currNode->sons[i]->key==key){
			*found=1;
			*location=i;
			return currNode;
		}
		//check if should be before or after the son in the
		if (currNode->sons[i]->key<key)
		{
			*location=i+1;
			return currNode;
		}
		*location=i;
		return currNode;
	}
	else {
		release_shared_lock(&(currNode->sons[0]->vxLock));
		if (i){
			i--;
		}
		//remember the last location of the son.
		*location=i;
		return findForInsert(currTree,currNode->sons[i],key,location,found,0);

	}
}

/*----------------------------------------------------------------
findElm function: implementing the FIND operation
Input:  currTree - the current tree
	    key - the key we are searching
Output: value - the value if the node with the given key
Return value: 0 if the key was found, -1 otherwise
Description: call the serch function findNode
-----------------------------------------------------------------*/
int findElm(tree currTree,int key,char* value){
	int location;
	int found = 0;
	vertex father;
	vertex root;

	//get root
	get_read_lock(&(currTree->treeLock));
	root = currTree->root;
	if (root != NULL){
		get_read_lock(&(currTree->root->vxLock));
	}
	release_shared_lock(&(currTree->treeLock));

	//find element
	(*value) = findNode(root,key,&found);
	if (!found){
		return -1;
	}
	return 0;
}

/*----------------------------------------------------------------
findNode function: finding leafs by key for the findElm function
Input:   currNode - the current node in the search
	     key - the key we are searching
Output: found - 1 if we found a node with the given key, 0 - otherwise
Return value: the value if the node with the given key
Description: looking for the leaf with the given key.
		every node in the serch path is locked as a Reader
		until the next node in the search path is caught
-----------------------------------------------------------------*/
char findNode(vertex currNode, int key, int* found){

	int i,leafKey;
	char value;
	vertex nextVertex;

	//invalid input
	if ((NULL==currNode) || (NULL==found)){
		return '0';
	}

	//init
	*found = 0;
	//in case the root is a leaf
	if (currNode->numSons == 0){
		leafKey = currNode->key;
		value = currNode->value;
		release_shared_lock(&(currNode->vxLock));
		if (leafKey == key){
			(*found) = 1;
		}
		return value;
	}

	//in case the
	for(i=0;i<(currNode->numSons-1);i++){
		if(key<currNode->keys[i]){
			break;
		}
	}
	//if is not in the first key, means that it is
	//bigger than the current child or equal to it
	if (i){
		i++;
	}
	//if it is the father
	get_read_lock(&(currNode->sons[0]->vxLock));
	if(currNode->sons[0]->isLeaf){
		release_shared_lock(&(currNode->sons[0]->vxLock));
		if ((i>0) && (currNode->sons[i-1]->key==key)){
			*found=1;
			value=currNode->sons[i-1]->value;
			release_shared_lock(&(currNode->vxLock));
			return value;
		}
		if(NULL==currNode->sons[i]){
			release_shared_lock(&(currNode->vxLock));
			return '0';
		}
		//check if it has the same son already
		if (currNode->sons[i]->key==key){
			*found=1;
			value=currNode->sons[i]->value;
			release_shared_lock(&(currNode->vxLock));
			return value;
		}
		release_shared_lock(&(currNode->vxLock));
		return '0';
	}
	else {
		release_shared_lock(&(currNode->sons[0]->vxLock));
		if (i){
			i--;
		}
		nextVertex = currNode->sons[i];
		get_read_lock(&(nextVertex->vxLock));
		release_shared_lock(&(currNode->vxLock));
		return findNode(nextVertex,key,found);
	}
}


/*----------------------------------------------------------------
releaseSharedPath function:
Input:  currTree - the current tree
	    currNode - the current node in the release
Return value: NONE
Description: frees the MayWrite lockes in the search path
-----------------------------------------------------------------*/
void releaseSharedPath(tree currTree,vertex currNode)
{
	vertex father = NULL;
	if (currNode == currTree->root){
		release_shared_lock(&(currNode->vxLock));
		release_shared_lock(&(currTree->treeLock));
	}
	else {
		father=currNode->father;
		release_shared_lock(&(currNode->vxLock));
		releaseSharedPath(currTree,father);
	}
}

/*----------------------------------------------------------------
releaseExclusivePath function:
Input:  currTree - the current tree
	    currNode - the current node in the release
Return value: NONE
Description: frees the Write lockes in the search path
-----------------------------------------------------------------*/
void releaseExclusivePath(tree currTree,vertex currNode)
{
	vertex father = NULL;
	if (currNode == currTree->root){
		release_exclusive_lock(&(currNode->vxLock));
		release_exclusive_lock(&(currTree->treeLock));
	}
	else {
		father=currNode->father;
		release_exclusive_lock(&(currNode->vxLock));
		releaseExclusivePath(currTree,father);
	}
}

/*----------------------------------------------------------------
fixTree function:
Input:  currTree - the current tree
	    currNode - the lowest node in the search path
Return value: 0 on success
			 -1 on if the given key is already in the tree
			 -2 on errors (for instance allocation error)
Description: fixes an unbalanced tree
-----------------------------------------------------------------*/
int fixTree(tree currTree,vertex currNode){

	vertex newVertex = NULL;
	vertex father = NULL;
	int i,j,finishFlag=0;

	//in case a split is not needed
	if(currNode->numSons <= TREE_VALUE){
		father = currNode->father;
		if (father==NULL) {
			release_exclusive_lock(&(currNode->vxLock));
			release_exclusive_lock(&(currTree->treeLock));
		}
		else {
			releaseExclusivePath(currTree,currNode);
		}
		return 0;
	}

	//splitting the node
	if(-1 == createVertex(&newVertex)){
		releaseExclusivePath(currTree,currNode);
		return -2;
	}

	//the new brother gets the 5 higher sons
	currNode->numSons=(TREE_VALUE+1)/2;
	newVertex->numSons=(TREE_VALUE+1)/2;
	for(i=0;i<(TREE_VALUE/2);i++)
	{
		newVertex->sons[i]=currNode->sons[i+(TREE_VALUE+1)/2];
		//newVertex is not yet conected to the tree therefore,
		//we can perform operations on his sons without locking
		newVertex->sons[i]->father = newVertex;
		currNode->sons[i+(TREE_VALUE+1)/2]=NULL;
		newVertex->keys[i]=currNode->keys[i+(TREE_VALUE+1)/2];
		currNode->keys[i+(TREE_VALUE+1)/2]=0;
	}
	newVertex->sons[i]=currNode->sons[i+(TREE_VALUE+1)/2];
	newVertex->sons[i]->father=newVertex;
	currNode->sons[i+(TREE_VALUE+1)/2]=NULL;

	//add the new key to the father
	//there is no father, create one
	father = currNode->father;
	if (NULL == father)
	{

		if(-1==createVertex(&father)){
			release_exclusive_lock(&(currNode->vxLock));
			release_exclusive_lock(&(currTree->treeLock));
			return -2;
		}
		father->sons[0] = currNode;
		father->sons[1] = newVertex;
		father->keys[0] = currNode->keys[(TREE_VALUE)/2];
		currNode->keys[(TREE_VALUE)/2] = 0;
		father->numSons = 2;
		newVertex->father = father;
		currNode->father = father;

		release_exclusive_lock(&(currNode->vxLock));

		//updating the root of the tree
		currTree->root = father;
		release_exclusive_lock(&(currTree->treeLock));

		return 0;
	}
	else {
		newVertex->father = father;
		//find currNode's location in father
		for(i=0;i<TREE_VALUE;i++){
			if (father->sons[i]==currNode){
				break;
			}
		}
		//newVertex should be inserted to the i+1 location-make a place
		for(j=father->numSons;(i+1)<j;j--){
			father->sons[j]=father->sons[j-1];
			if(0!=(j-1)){
				father->keys[j-1]=father->keys[j-2];
			}
		}
		father->keys[i]=currNode->keys[TREE_VALUE/2];
		currNode->keys[TREE_VALUE/2]=0;

		//release the current vertex, the father is locked for writing
		release_exclusive_lock(&(currNode->vxLock));
		father->sons[i+1]=newVertex;
		father->numSons++;
		//release_exclusive_lock(&(currNode->vxLock));
		return fixTree(currTree,father);
	}

	return 0;
}

/*----------------------------------------------------------------
addElm function: implementing the INSERT operation
Input:  currTree - the current tree
	    key - the key of the new node
        value - the value of the new node
Return value: 0 on success
			 -1 on if the given key is already in the tree
			 -2 on errors (for instance allocation error)
Description: -call the serch function findForInsert
			 -adds the new node to the tree
			 -calls fixTree to fix an unbalanced tree
-----------------------------------------------------------------*/
int addElm(tree currTree,int key,char value){

	int i,j,tempKey,res;
	vertex currRoot,father;
	vertex tempVertex = NULL;
	vertex newLeaf;
	int found, upgradeMade=0;

	//case of empty tree
	if(currTree->isEmpty){
		get_write_lock(&(currTree->treeLock));
			//checking if the tree is still empty
			if(currTree->isEmpty){
				currTree->isEmpty = 0;
				//create new leaf
				if(-1 == createLeaf(&(tempVertex),key,value)){
					release_exclusive_lock(&(currTree->treeLock));
					return -2;
				}
				//updateFlag root
				currTree->root = tempVertex;
				release_exclusive_lock(&(currTree->treeLock));
				return 0;
			}
		release_exclusive_lock(&(currTree->treeLock));
	}

	//case the root is a leaf
	get_may_write_lock(&(currTree->treeLock));
	if (0 == currTree->root->numSons){
		currRoot=currTree->root;
		get_read_lock(&(currRoot->vxLock));
		tempKey = currRoot->key;
		release_shared_lock(&(currRoot->vxLock));

		if(tempKey == key){
			release_shared_lock(&(currTree->treeLock));
			return -1;
		}

		//changing the root
		upgrade_may_write_lock(&(currTree->treeLock));

			//create new vertex for the root
			if(-1 == createVertex(&(currTree->root))){
				release_exclusive_lock(&(currTree->treeLock));
				return -2;
			}

			//inserting the new leaf
			if(tempKey > key){ //before the old leaf
				currTree->root->keys[0] = tempKey;
				i=0;
			}
			else{ //after the old leaf
				currTree->root->keys[0] = key;
				i=1;
			}

			//creating the new leaf
			if(-1 == createLeaf(&(currTree->root->sons[i]),key,value)){
				release_exclusive_lock(&(currTree->treeLock));
				return -2;
			}
			//inserting the old one to the root
			currTree->root->sons[1-i] = currRoot;
			//update the old one father
			currRoot->father = currTree->root;
			//update the new one father
			currTree->root->sons[i]->father=currTree->root;
			currTree->root->numSons=2;

		release_exclusive_lock(&(currTree->treeLock));
		return 0;
	}
	else{
		release_shared_lock(&(currTree->treeLock));
	}
	//case the root is not NULL or leaf
	tempVertex = findForInsert(currTree,currTree->root,key,&i,&found,1);

	//error check
	if(1 == found){
		releaseSharedPath(currTree,tempVertex);
		return -1;
	}
	if (NULL == tempVertex) {
		return -2;
	}

	//creating  a new leaf
	if(-1==createLeaf(&(newLeaf),key,value)){
		releaseSharedPath(currTree,tempVertex);
		return -2;
	}

	//upgrading the path that might be changed.
	if (tempVertex->numSons == TREE_VALUE){
		upgradePath(currTree,tempVertex);
		upgradeMade=1;
	}
	else{
		upgrade_may_write_lock(&(tempVertex->vxLock));
	}

	//insert the son, afterwards check if number of sons has exceeded
	for(j=tempVertex->numSons;i<j;j--){
		tempVertex->sons[j]=tempVertex->sons[j-1];
		if(0!=(j-1)){
			tempVertex->keys[j-1]=tempVertex->keys[j-2];
		}
	}
	tempVertex->sons[i]=newLeaf;
	tempVertex->sons[i]->father=tempVertex;
	if(0!=i){
		tempVertex->keys[i-1]=key;
	}
	else{
		tempVertex->keys[0]=tempVertex->sons[1]->key;
	}
	(tempVertex->numSons)++;

	res=0;
	if (upgradeMade)
	{
		res=fixTree(currTree,tempVertex);
	}
	else{
		father=tempVertex->father;
		release_exclusive_lock(&(tempVertex->vxLock));
		if (NULL==father) {
			release_shared_lock(&(currTree->treeLock));
		}
		else {
			releaseSharedPath(currTree,father);
		}
	}

	return res;
}

/*----------------------------------------------------------------
upgradePath function:
Input:  currTree - the current tree
	    lastNode - the lowest node in the search path
Return value: NONE
Description: upgrades all the nodes in the search path of lastNode
-----------------------------------------------------------------*/
void upgradePath(tree currTree,vertex lastNode)
{
	int index=0,i;
	vertex currNode=lastNode;

	//first store all the nodes in the array
	while (currNode!=NULL){
		upgradePathArr[index]=currNode;
		currNode=currNode->father;
		index++;
	}

	//in case the given node was NULL
	if (0==index){
		return;
	}

	upgrade_may_write_lock(&(currTree->treeLock));

	//now upgrade from top to buttom
	for(i=(index-1);i>=0;i--){
		upgrade_may_write_lock(&(upgradePathArr[i]->vxLock));
	}



}

/*----------------------------------------------------------------
freeAllSons function:
Input:  currNode - the current node in the free process
Return value: NONE
Description: recursively freeing the tree
-----------------------------------------------------------------*/
void freeAllSons(vertex currNode)
{
	int i;
	if (currNode==NULL) {
		return;
	}

	for(i=0; i < currNode->numSons; i++){
		if (0 == currNode->sons[i]->isLeaf){
			freeAllSons(currNode->sons[i]);
		}
		lock_destroy(&(currNode->sons[i]->vxLock));
		free(currNode->sons[i]);
	}

	return;
}

/*----------------------------------------------------------------
freeTree function: freeing a tree
Input:  tr - the current tree
Return value: NONE
Description: - calls the freeAllSons to free all the sons
			 - frees the root
-----------------------------------------------------------------*/
void freeTree(tree tr)
{
	if (tr!=NULL)
	{
		freeAllSons(tr->root);
	}
	free(tr->root);
	free(tr);
}



