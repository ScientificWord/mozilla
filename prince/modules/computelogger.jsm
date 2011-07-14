var EXPORTED_SYMBOLS = ["msiComputeLogger"];

function jsdump(str)
{
  Components.classes['@mozilla.org/consoleservice;1']
            .getService(Components.interfaces.nsIConsoleService)
            .logEngineStringMessage(str);
}



var msiComputeLogger = 
{
  logMMLSent:      true,
  logMMLReceived:  true,
  logEngSent:      true,
  logEngReceived:  true,
  engine: null,

  Sent: function(name,expr)
  {
    if (this.logMMLSent)
      jsdump("To engine: " + name + 
        ": ======================================\n" + expr + "\n");
  },
  
  Sent4: function(name,expr,arg1,arg2)
  {
    if (this.logMMLSent)
      jsdump("To engine: " + name + 
        ": ======================================\n" + expr + 
        "\n" + arg1 + "\n" + arg2 + "\n");
  },
  
  LogEngineStrs: function()
  {
    if (this.logEngSent)
      jsdump("To engine: sent:  ===> " + 
        this.engine.getEngineSent() + "\n");
    if (this.logEngReceived)
      jsdump("From engine: rcvd: <===  " + 
        this.engine.getEngineReceived() + "\n");
  },
  
  Received: function(expr)
  {
    this.LogEngineStrs();
    if (this.logMMLReceived)
      jsdump(
        "Engine result: ======================================\n" + 
        expr + "\n");
  },
  
  Exception: function(e)
  {
    this.LogEngineStrs();
    if (this.logMMLReceived) {
      jsdump("Compute exception: !!!!!!!!!!!!\n");
    }
    jsdump(e);
    // separate pref for errors?
    jsdump("\Compute engine:  " + this.engine);
    jsdump("\n           errors:  " + this.engine.getEngineErrors() + "\n");
  },
  
  Init: function(engine, logMMLSent, logMMLReceived, logEngSent, logEngReceived)
  {
    this.engine = engine;
    this.logMMLSent = logMMLSent;
    this.logMMLReceived = logMMLReceived;
    this.logEngSent = logEngSent;
    this.logEngReceived = logEngReceived;
  },
  
  LogMMLSent: function(log)
  {
    this.logMMLSent = log;
  },
  
  LogMMLReceived: function(log)
  {
    this.logMMLReceived = log;
  },
  
  LogEngSent: function(log)
  {
    this.logEngSent = log;
  },
  
  LogEngReceived: function(log)
  {
    this.logEngReceived = log;
  }

};
