 #include "IotaWatt.h"

#define graphDir "graphs"
String hashName(const char* name);
char* fileType = nullptr;;

void setFileType(){
   delete fileType;
   if(server.hasArg(F("version")) && server.arg(F("version")).equals("2")){
     fileType = charstar(".jsn");
   } else {
     fileType = charstar(".txt");
   }
}

void handleGraphCreate(){
  setFileType();
  File graphFile = SD.open(graphDir);
  if(graphFile){
    if( ! graphFile.isDirectory()){
      graphFile.close();
      SD.remove(graphDir);
      SD.mkdir(graphDir);
    }
  }
  else {
    SD.mkdir(graphDir);
  }
  
  trace(T_uploadGraph,11);
  DynamicJsonBuffer Json;
  JsonObject& graph = Json.parseObject(server.arg("data"));
  String filePath = graphDir;
  String fileName = hashName(graph["name"].as<char*>());
  filePath += "/" + fileName + fileType;
  SD.remove(filePath);
  graphFile = SD.open(filePath, FILE_WRITE);
  if( ! graphFile){
    server.send(500, txtPlain_P, F("Couldn't create file."));
    return;
  } 
  graphFile.write(server.arg("data").c_str());
  graphFile.close();
  String response(F("{\"success\":true,\"message\":\"graph saved id:"));
  response += fileName + "\"}";
  server.send(200, appJson_P, response);  
  return;
}

void handleGraphDelete(){
  setFileType();
  String filePath = graphDir;
  String fileName = server.arg("id");
  filePath += "/" + fileName;
  if(!fileName.endsWith(fileType)){
    fileName += fileType;
  }
  SD.remove(filePath);
  server.send(200, txtJson_P, F("{\"success\":true,\"message\":\"deleted\"}"));
}

void handleGraphGetall(){
  setFileType();
  File directory = SD.open(graphDir);
  File graphFile;
  if( !(directory) || !(directory.isDirectory()) || !(graphFile = directory.openNextFile())){
    if(directory){
      directory.close();
    }
    server.send(200, appJson_P, "[]");
    return;
  }
  String response = "[";
  while(graphFile){
    String name(graphFile.name());
    name = name.substring(name.indexOf('.'));
    if(name.equalsIgnoreCase(fileType)){
      if( ! response.endsWith("[")) response += ',';
      uint32_t fileSize = graphFile.size();
      char* bufr = new char[fileSize];
      graphFile.read((uint8_t*)bufr, fileSize);
      bufr[fileSize-1] = 0;
      response += bufr;
      delete[] bufr;
      graphFile.close();
      response += ",\"id\":\"";
      response += graphFile.name();
      response += "\"}";
    }
    graphFile.close();
    graphFile = directory.openNextFile();
  }
  response += ']';
  server.send(200, appJson_P, response);
  directory.close();
  return;
}
