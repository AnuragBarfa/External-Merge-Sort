#include "common.h"
#include "ExtMergeSort.h"

//creates initial runs of 1 page size
void ExtMergeSort :: firstPass(DiskFile &inputFile, MainMemory &memory){
	
	int frame = -1;
	// load each page to main memory frame and sort
	for(int i = 0; i < inputFile.totalPages; i++){
		frame = memory.loadPage(inputFile, i);
		this->sortFrame(memory, frame);
		memory.writeFrame(inputFile, frame, i);
		memory.freeFrame(frame);
	}
	
	runSize = 1;
	totalPass = 1;
	totalRuns = inputFile.totalPages;
	cout << "First Pass Performed" << endl;
	inputFile.writeDiskFile(); //print file to cout
}

void ExtMergeSort :: firstPass(DiskFile &inputFile, MainMemory &memory, int B){
	vector <int> frames;//stores index of the frames alloted from main memory
	for(int i = 0; i < inputFile.totalPages; i = i + B){
		frames.clear();
		//load B pages to available B frames and store the those frames index
		//make sure to handle if loadPage return -1 no frame can be assign 
		for(int j = 0; (i + j < inputFile.totalPages) && (j < B); j++){
			frames.push_back(memory.loadPage(inputFile, i + j));
		}
		cout << "hello" << endl;
		//trying to rearrange content of frame so that every frame is completly filled execpt one frame atmost
		for(int j = 0; j < frames.size()-1; j++){
			while (j < frames.size() - 1 && memory.getValidEntries(frames[j]) != MEM_FRAME_SIZE){
				//this while loop try to fill current frame with the content of last frame if thier is some space available in current frame
				int temp = memory.getVal(frames[frames.size() -1], 0);//if valid frame then contain value stored at index 0 of the frame
				if (temp == -1){
					//if last frame is notvalid(it's content) then remove it from the list of frames required
					frames.resize(frames.size() - 1);
					continue;
				}
				memory.setVal(frames[j], memory.getValidEntries(frames[j]), temp);//put the value in current frame in available space
				memory.invalidate(frames[frames.size()-1], 0);//invalidate the entry in last frame copied to current frame from lasst frame 
				//frame is not invalidate after every intry of frame has been invalidate temp will never become -1 
			}
			cout << "hello1" << endl;
			
		}
		cout <<" hello2" << endl; 
		int num = 0;
		int validentries = (frames.size()-1)*MEM_FRAME_SIZE + memory.getValidEntries(frames[frames.size() -1]);//total number of valid entries in main memory corresponding to B pages to be sorted
		cout << validentries << endl;
		while(num < validentries - 1){
			//put correct entry corresponding to index num in the final sorted order of data 
			//selection sort procedure
			int minindex = 0;//index corresponding to minimum value in the unordered data
			int minm = INT_MAX;//minimum among the unorder data
			for(int j = num; j < validentries; j++){
				//to find the minimum in unordered data
				if (memory.getVal(frames[j/MEM_FRAME_SIZE], j%MEM_FRAME_SIZE) < minm){
					minm = memory.getVal(frames[j/MEM_FRAME_SIZE], j%MEM_FRAME_SIZE);
					minindex = j;
				}
			}
			cout << minm << " ";
			if (minm == INT_MAX)
				//when whole content of unordered data is INT_MAX or invalidentries then we are done no sorting left to be done
				break;
			swap(memory.data[frames[num/MEM_FRAME_SIZE]].arr[num%MEM_FRAME_SIZE], memory.data[frames[minindex/MEM_FRAME_SIZE]].arr[minindex%MEM_FRAME_SIZE]);
			num++;
		}
		for(int j = 0; j < frames.size(); j++){
			//should consider the cases when B page are not filled completly and lead to deacrese in required frame after arrangment than B so while writing back less pages will be writtten back so some invalid pages will exist and then should be considerd in second run
			memory.writeFrame(inputFile, frames[j], i + j);
			memory.freeFrame(frames[j]);
		}
		runSize = B;
		totalPass = 1;
		totalRuns = inputFile.totalPages/B;
	}
}

//sorts each frame
void ExtMergeSort :: sortFrame(MainMemory &memory, int f){
	sort(memory.data[f].arr.begin(), memory.data[f].arr.begin()+memory.getValidEntries(f));
}

//Performs merging of 2 runs
void ExtMergeSort :: merge(DiskFile &inputFile, MainMemory &memory, int leftStart, int mid, int rightEnd){

	int finalRunSize = rightEnd - leftStart + 1;
	DiskFile tempFile(finalRunSize);

	bool flag = false;
	int currPage = 0;
	int l = leftStart;
	int r = mid + 1;

	int leftFrame = memory.loadPage(inputFile, l);
	int rightFrame = memory.loadPage(inputFile, r);
	int resFrame = memory.getEmptyFrame();	//frame to store result
	if(leftFrame == -1 || rightFrame == -1 || resFrame == -1){
		cout << "Can't proceed further due to unavailability of memory or invalid Page access" << endl;
		exit(1); 
	}

	int leftIndex = 0;
	int rightIndex = 0;
	int resIndex = 0;

	while(l <= mid && r <= rightEnd){
		if(leftIndex < memory.getValidEntries(leftFrame) && rightIndex < memory.getValidEntries(rightFrame)){	
			int x = memory.getVal(leftFrame, leftIndex);
			int y = memory.getVal(rightFrame, rightIndex); 
			if( x < y){
				memory.setVal(resFrame, resIndex, x);
				flag = true;
				leftIndex++;
			}
			else{
				flag = true;
				memory.setVal(resFrame, resIndex, y);
				rightIndex++;
			}
			resIndex++;
			if(resIndex == MEM_FRAME_SIZE){
				memory.writeFrame(tempFile, resFrame, currPage);
				flag = false;
				memory.freeFrame(resFrame);
				resFrame = memory.getEmptyFrame();
				currPage++; 
				resIndex = 0;
			}
		}

		if(leftIndex == memory.getValidEntries(leftFrame)){
			memory.freeFrame(leftFrame);
			l++;
			if(l <= mid){
				leftFrame = memory.loadPage(inputFile, l);
				leftIndex = 0;
			}	
		}

		if(rightIndex == memory.getValidEntries(rightFrame)){
			memory.freeFrame(rightFrame);
			r++;
			if(r <= rightEnd){
				rightFrame = memory.loadPage(inputFile, r);
				rightIndex = 0;
			}	
		}
	}
	while(l <= mid){
		int x = memory.getVal(leftFrame, leftIndex);
		memory.setVal(resFrame, resIndex, x);
		flag = true;
		leftIndex++;
		resIndex++;
		if(resIndex == MEM_FRAME_SIZE){
			memory.writeFrame(tempFile, resFrame, currPage);
			flag = false;
			memory.freeFrame(resFrame);
			resFrame = memory.getEmptyFrame();
			currPage++; 
			resIndex = 0;
		}
		if(leftIndex == memory.getValidEntries(leftFrame)){
			memory.freeFrame(leftFrame);
			l++;
			if(l <= mid){
				leftFrame = memory.loadPage(inputFile, l);
				leftIndex = 0;
			}	

		}
	}
	while(r <= rightEnd){
		int x = memory.getVal(rightFrame, rightIndex);
		memory.setVal(resFrame, resIndex, x);
		flag = true;
		rightIndex++;
		resIndex++;
		if(resIndex == MEM_FRAME_SIZE){
			memory.writeFrame(tempFile, resFrame, currPage);
			flag = false;
			memory.freeFrame(resFrame);
			resFrame = memory.getEmptyFrame();
			currPage++; 
			resIndex = 0;
		}
		if(rightIndex == memory.getValidEntries(rightFrame)){
			r++;
			memory.freeFrame(rightFrame);
			if(r <= rightEnd){
				rightFrame = memory.loadPage(inputFile, r);
				rightIndex = 0;
			}	

		}
	}
	if(flag)
		memory.writeFrame(tempFile, resFrame, currPage);
	memory.freeFrame(resFrame);
	memory.freeFrame(leftFrame);
	memory.freeFrame(rightFrame);
	inputFile.DiskFileCopy(tempFile, leftStart, rightEnd);

}

void ExtMergeSort :: Bmerge(DiskFile &inputFile, MainMemory &memory, int leftStart, int B){
	int finalRunSize = ((B-2)*this->runSize)/2;
	DiskFile tempFile(finalRunSize);
	int currPage = 0;
	// int resFrame = memory.getEmptyFrame();
	vector < vector <int> > frames(B/2, vector <int> (2));//buffer of size two for each frame in main memmory
	for(int i = 0; i < (B - 2)/2; i++){
		if (leftStart + i*this->runSize < inputFile.totalPages)
			frames[i][0] = memory.loadPage(inputFile, leftStart + i*this->runSize);
		else
			frames[i][0] = memory.getEmptyFrame();
		if (leftStart + i*this->runSize + 1< inputFile.totalPages)
			frames[i][1] = memory.loadPage(inputFile, leftStart + i*this->runSize+1);
		else
			frames[i][1] = memory.getEmptyFrame();
	}
	frames[B/2 - 1][0] = memory.getEmptyFrame();
	frames[B/2 - 1][1] = memory.getEmptyFrame();
	vector <int> active(B/2, 0);//out of two frames in buffer which one is currenlty active
	vector <int> indexes(B/2, 0);//index which is not readed yet in frame whose index is stored at ith index of vector frames
	int minm = INT_MAX;//current minimum value out of the unordered data left among all the frame having the data
	int minindex = 0;//index in vector frames pointing to a frame having minimum value in current go out of unordered values 
	cout << "frame  " <<  frames.size()<< endl;
	for(;;){
		minm = INT_MAX;
		for(int i = 0; i < frames.size()-1; i++){
			int index = indexes[i];
			if (index/MEM_FRAME_SIZE >= this->runSize || index%MEM_FRAME_SIZE >= memory.getValidEntries(frames[i][active[i]]))
				continue;
			if (minm > memory.getVal(frames[i][active[i]], index%MEM_FRAME_SIZE)){
				minm = memory.getVal(frames[i][active[i]], index%MEM_FRAME_SIZE);
				minindex = i;
				cout << minm << " " << i << " " << active[i] << endl;
			}
		}
		cout << minm << endl;
		if (minm == INT_MAX)
			break;
		memory.setVal(frames[B/2-1][active[B/2-1]], indexes[B/2-1], minm);
		indexes[minindex]++;
		if (indexes[minindex]%MEM_FRAME_SIZE == 0){
			//if the minimum entry was the last entry of the page having minimum valueamong all
			if (minindex*this->runSize + indexes[minindex]/MEM_FRAME_SIZE + 1>= inputFile.totalPages){
				frames[minindex][active[minindex]] = memory.getEmptyFrame();
			}
			else{
				memory.freeFrame(frames[minindex][active[minindex]]);			
				frames[minindex][active[minindex]] = memory.loadPage(inputFile, minindex*this->runSize + indexes[minindex]/MEM_FRAME_SIZE+1);
			}
			active[minindex] = (active[minindex]+1)%2;
		}
		indexes[B/2-1]++;
		if (indexes[B/2-1] == MEM_FRAME_SIZE){
			memory.writeFrame(tempFile, frames[B/2-1][active[B/2-1]], currPage);
			currPage++;
			memory.freeFrame(frames[B/2-1][active[B/2-1]]);
			frames[B/2-1][active[B/2-1]] = memory.getEmptyFrame();
			active[B/2-1] = (active[B/2-1]+1)%2;
			indexes[B/2-1] = 0;			
		}	
	}
	if (indexes[B/2-1]){
		memory.writeFrame(tempFile, frames[B/2-1][active[B/2-1]], currPage);
	}
	inputFile.DiskFileCopy(tempFile, leftStart, leftStart + finalRunSize - 1);
	for(int i = 0; i < frames.size(); i++){
			memory.freeFrame(frames[i][active[0]]);
			memory.freeFrame(frames[i][active[1]]);
		
	}
	
}

//Performs 2 way merge sort on inputFile using memory
void ExtMergeSort :: twoWaySort(DiskFile &inputFile, MainMemory &memory){
	
	if(memory.totalFrames < 3)
		cout << "Error: Two way merge sort requires atleast 3 frames" << endl; 
	
	this->firstPass(inputFile, memory);

	int leftStart;
	
	for(this->runSize = 1; this->runSize < inputFile.totalPages; this->runSize *= 2){
		cout << "runSize: " << this->runSize << endl;
		for(leftStart = 0; leftStart < inputFile.totalPages-1; leftStart += 2*this->runSize){
			int mid = leftStart + this->runSize-1;
			int rightEnd = min(leftStart + 2*this->runSize-1, inputFile.totalPages-1);
			cout << "calling merge for < " << leftStart <<", " << mid << ", " << rightEnd << " >" << endl;
			this->merge(inputFile, memory, leftStart, mid, rightEnd);
		}
		totalPass++;
	}

	cout << "Total Passes required: " << totalPass << endl;
}

void ExtMergeSort :: BWaySort(DiskFile &inputFile, MainMemory &memory, int B){
	this->firstPass(inputFile, memory, B);
	cout << "Pass1 done" << endl;
	inputFile.writeDiskFile();
	int leftStart;
	for(this->runSize = B; this->runSize < inputFile.totalPages; this->runSize *= (B/2-1)){
		cout << "runSize: " << this->runSize << endl;
		for(leftStart = 0; leftStart < inputFile.totalPages; leftStart += (B/2-1)*this->runSize){
			this->Bmerge(inputFile, memory, leftStart,B);
		}
		totalPass++;
	}
	cout << runSize << endl;
	
}

