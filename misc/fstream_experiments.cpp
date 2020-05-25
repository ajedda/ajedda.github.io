#include <iostream> 
#include <fstream>
#include <sstream> 
#include <cstdio>
#include <boost/program_options.hpp>

using namespace boost::program_options;

int main(int argc, char** argv) 
{
  
  std::string input_filename; 
  std::string output_filename; 
  int which_line{1};
  char delim{'\n'}; 
	
  try
  {
    options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("input,i", value<std::string>(&input_filename)) 
      ("output,o", value<std::string>(&output_filename))
      ("delim,d", value<char>(&delim)->default_value('\n'))
      ("line,l", value<int>(&which_line)); 

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);
    if (vm.count("help")) { std::cout << desc << '\n'; } 
 	} 
  catch (const error& ex) { std::cerr << ex.what() << '\n'; }	
  if (input_filename.empty() || output_filename.empty()) { std::cout << "can't work it!\n"; }
  
   
  std::ifstream is{input_filename}; 
  std::fstream  os{output_filename, std::ofstream::out}; 
  if (!is) { std::cout << "could not read: " << input_filename << '\n'; }
  if (!os) { std::cout << "could not read: " << output_filename << '\n'; }   

  for (int i{1}; (is) ; i++)
  { 
    if (i == which_line) 
    {  
        if (is.peek() == delim) { os.put(is.get()); }
        
        for (  ; (is) && is.peek() != delim ; is.get() ) {}
        if (is.peek() == delim) { is.get(); }
        
        is.get(*os.rdbuf(), EOF); 
		    break; 	
   } 
   
   for ( ; (is) && (is.peek() == delim);   os.put(is.get()) ) {}   
   is.get(*os.rdbuf(), delim);
  }
  
} 

