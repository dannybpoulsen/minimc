namespace MiniMC {
  namespace Support {
    template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };
  }
}
