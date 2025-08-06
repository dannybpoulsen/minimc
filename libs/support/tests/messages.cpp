#include "minimc/support/feedback.hpp"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

int main () {
  MiniMC::Support::MessagePipelineBuilder builder;
  builder.add<MiniMC::Support::StreamHandler<MiniMC::Support::Severity::Error>> (MiniMC::IO::os_ostream::err());
  builder.add<MiniMC::Support::StreamHandler<MiniMC::Support::Severity::Warning>> (MiniMC::IO::os_ostream::err());
  builder.add<MiniMC::Support::StreamHandler<MiniMC::Support::Severity::Info>> (MiniMC::IO::os_ostream::out());
  builder.add<MiniMC::Support::ProgressStreamHandler> (MiniMC::IO::os_ostream::out());
  MiniMC::Support::MessageSink::setDefaultSink (builder.build());


  MiniMC::Support::Messager messager;
  MiniMC::Support::AsyncExecutor exec;
  auto func = []() {
    MiniMC::Support::Messager messager;
    
    for (int i = 0; i < 100; i++) {
      {
	MiniMC::Support::SubProgresSenderClearer<int> clearer (i);
	std::this_thread::sleep_for(1s);
	messager << MiniMC::Support::TProgress<int> {i};
	
      }
      std::this_thread::sleep_for(1s);
      
    }
  };
  exec.execute (messager,func);
  
  
}
