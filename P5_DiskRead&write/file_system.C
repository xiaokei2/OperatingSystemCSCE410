/* 
File: file_system.C

Author: Shaobo Wang
Department of Computer Science
Texas A&M University
Date  : 10/04/05

Description: File System.


*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file_system.H"
#include "assert.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F i l e */
/*--------------------------------------------------------------------------*/
File::File(){}
File::File(unsigned int file_id, FileSystem *FILE_SYSTEM){
        
        if (FILE_SYSTEM->LookupFile(file_id, this))
            Console::puts("there is a file here\n"); //found File and reinialize 
        else if (FILE_SYSTEM->CreateFile(file_id)){
            cur_block=0;
            cur_pos=0;
            file_size=0;
            block_nums=NULL;
            
        }

   }


unsigned int File::Read(unsigned int _n, unsigned char * _buf) {
  // Write your code and remove assert below and return proper value
  unsigned int char_read=0;
  while((_n-char_read)>0)
  {
	if(EoF())
	   return 0;
	file_system->disk->read(block_nums[cur_block],disk_buf);
	while(cur_pos>500)
	{
		if(_n==1) 
	  	break;
        memcpy(_buf,disk_buf+12+cur_pos,1); // copy char array one by one
	//_n--;
	_buf++;
        char_read++;
	}
  }
	return char_read;
}

unsigned int File::Write(unsigned int _n, unsigned char * _buf) {
  // Write your code and remove assert below and return proper value
  unsigned int char_write = 0;

  while( (_n-char_write)> 500)
  {
	memcpy((void*)(disk_buf+12),_buf,500);
	file_system->disk->write(block_nums[cur_block],disk_buf);
	
        _n-=(500);
	char_write++;

	if(EoF())
	{
		return 0;
	//
	}
  }
	
	return char_write;
//  return 0;
}

void File::Reset() {
  // Write your code and remove assert below and return proper value
  cur_pos=0;
  cur_block =0;
}

void File::Rewrite() {
  // Write your code and remove assert below and return proper value
  
  while(cur_block < file_size){ //release memory 
	cur_block++;
	file_system->removeBlock(cur_block);
	}
	cur_block=0;
	cur_pos=0;
	location=0;
//	file_system->

}

BOOLEAN File::EoF() {
  // Write your code and remove assert below and return proper value
  if(block_nums==NULL || cur_block+1 == file_size && cur_pos+1==500) 
  	return TRUE;
  
  else
	return FALSE;
}

unsigned int File::get_file_id() {
  // Write your code and remove assert below and return proper value
  return file_id;
}
unsigned int File::get_position() {
  // Write your code and remove assert below and return proper value
  
  return cur_pos;
}
unsigned int File::get_size() {
  // Write your code and remove assert below and return proper value
 
  return block_size;
}

unsigned int File::get_inode()
{
	return inode_block_num;
}

void File::assign_file_id(unsigned int id){
 	file_id = id;
}
void File::assign_position(unsigned int pos){
	cur_pos = pos;
}
void File::assign_size(unsigned int size){
	file_size = size;
}
void File::assign_inode_num(unsigned int inode){
	inode_block_num = inode;
}

void File::assign_block_nums(unsigned int *file)
{
	block_nums=file;
}
/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
  // Write your code and remove assert below and return proper value
  current_block=0;
  num_files=0;

}

BOOLEAN FileSystem::Mount(SimpleDisk * _disk) {
  // Write your code and remove assert below and return proper value
	_disk->read(0,disk_buf);
	Block*block = (Block*)disk_buf;
	num_files=block->block_size;
	for(int i=0;i<num_files;i++)
	{
	disk->read(0,disk_buf);
	File* new_file= new File();
	Block*block = (Block*)disk_buf;	
	new_file->assign_size( block->block_size);
	new_file->assign_file_id(block->block_id);
			
	files[num_files++]=*new_file;
	}
}

BOOLEAN FileSystem::Format(SimpleDisk * disk, unsigned int _size) {
  // Write your code and remove assert below and return proper value
  //disk=_disk;
  	memset(disk_buf,0,512); //Wipes any file system from the given disk
  	for(int i =0;i<10000;++i)
		disk->write(i,disk_buf);
  	Block*block = (Block*)disk_buf;
  	block->free=1;
  	block->block_size=0;
  	disk->write(0,disk_buf);// write 0 to disk_buff
  
  return false;
}

BOOLEAN FileSystem::LookupFile(int _file_id, File * _file) {
  // Write your code and remove assert below and return proper value
          unsigned int i=0;
        for (i=0;i<num_files+1;++i){
            if (files[i].get_file_id()==_file_id){
                *_file=files[i];
                return TRUE;
		}
	}
        return FALSE;
}


BOOLEAN FileSystem::CreateFile(int _file_id) {
       File* newFile=(File*) new File();

        if (LookupFile(_file_id,newFile)){

            return FALSE;
            }
        newFile->assign_file_id(_file_id);
        newFile->assign_size(0);
        //newFile->block_nums=NULL;


        newFile->assign_inode_num(get_block());//get any free block
        disk->read(newFile->get_inode(),disk_buf);//load block in buffer

        Block*block = (Block*)disk_buf;
        block->free=1;//set block to used
        block->block_size=0;//
        block->block_id=_file_id;
        disk->write(newFile->get_inode(),disk_buf);
        files[num_files++]=*newFile;
        return TRUE;

}



BOOLEAN FileSystem::DeleteFile(int _file_id) {
  // Write your code and remove assert below and return proper value
  File* newFile= (File*) new File();
  if(LookupFile(_file_id,newFile))
	{
	return FALSE;
	}
   newFile->assign_file_id(_file_id);
   newFile->assign_size(0);
   newFile->assign_block_nums(NULL);
   
   newFile->Rewrite();
  // newFile->assign
   newFile->assign_inode_num(get_block());
	
}

unsigned int FileSystem::get_size() {
  // Write your code and remove assert below and return proper value
  
  return size;
}

void FileSystem::removeBlock(unsigned int block_num)
{
	disk->read(block_num,disk_buf);
	Block*block = (Block*)disk_buf;
	disk->write(block_num,disk_buf);
}

unsigned int FileSystem::get_block()
{
	disk->read(current_block,disk_buf);
	Block*block = (Block*)disk_buf;
	while(block->free==1){
		++current_block;
		disk->read(current_block,disk_buf);	
	}
	disk->read(current_block,disk_buf);

	block = (Block*)disk_buf;

	block->free=1;
	
	disk->write(current_block,disk_buf);
	return current_block;
		
}

