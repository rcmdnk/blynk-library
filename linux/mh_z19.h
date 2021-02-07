class MHZ19{
public:
   MHZ19(const char* serial);
   int getCO2();
private:
   char m_serial[32];
};
