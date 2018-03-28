
client_name			=	client
server_name			=	server

obj_dir				=	obj
bin_dir				=	bin

include_dirs		=	shared

shared_src_dirs		=	shared/log \
						shared/misc \
						shared/network \
						shared/transport

client_src_dirs		=	client $(shared_src_dirs)
server_src_dirs		=	server $(shared_src_dirs)

client_src_files	=	$(wildcard $(addsuffix /*.cpp, $(client_src_dirs)))
server_src_files	=	$(wildcard $(addsuffix /*.cpp, $(server_src_dirs)))

client_obj_files	=	$(patsubst %.cpp, $(obj_dir)/%.o, $(client_src_files))
server_obj_files	=	$(patsubst %.cpp, $(obj_dir)/%.o, $(server_src_files))

client_app			=	$(bin_dir)/$(client_name)
server_app			=	$(bin_dir)/$(server_name)

link_flags			=	-lstdc++ -lpthread -lm
compile_flags		=	-std=c++1z -pedantic -Wall -Wextra -g

all:				$(client_app) $(server_app)

clean:
					rm -r -f $(bin_dir) $(obj_dir)

$(client_app):		build-client-repository $(client_obj_files)
					$(CXX) $(client_obj_files) $(link_flags) -o $@

$(server_app):		build-server-repository $(server_obj_files)
					$(CXX) $(server_obj_files) $(link_flags) -o $@

$(obj_dir)/%.o:		%.cpp
					$(CXX) -MD -c $(compile_flags) $(addprefix -I, $(include_dirs)) $< -o $@

build-client-repository:
					$(call make-client-repository)

build-server-repository:
					$(call make-server-repository)

define make-client-repository
					mkdir -p $(bin_dir); \
					for dir in $(client_src_dirs); \
					do \
						mkdir -p $(obj_dir)/$$dir; \
					done
endef

define make-server-repository
					mkdir -p $(bin_dir); \
					for dir in $(server_src_dirs); \
					do \
						mkdir -p $(obj_dir)/$$dir; \
					done
endef
