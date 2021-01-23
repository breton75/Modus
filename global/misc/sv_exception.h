#ifndef SV_EXCEPTION_H
#define SV_EXCEPTION_H

#include <QString>
#include <QException>

class SvException: public QException
{
public:

  enum ErrorType {
    NoError = 0,
    SomeError = 1
  };

  SvException()
  {
    this->error = "";
    this->type = NoError;
    this->code = 0;
  }

  SvException(int code)
  {
    this->error = "";
    this->type = code == 0 ? NoError : SomeError;
    this->code = code;
  }

  SvException(const QString& error, int code = 0)
  {
    this->error = error;
    this->type = error.isEmpty() ? NoError : SomeError;
    this->code = code;
  }

  SvException(ErrorType type, const QString& error, int code = 0)
  {
    this->error = error;
    this->type = type;
    this->code = code;
  }

  virtual ~SvException() throw() { }

  void raise(QString error) { this->error = error; throw *this; }
  void raise(int code, QString error = "") { this->code = code; this->error = error; throw *this; }
  void raise() { throw *this; }

  SvException& assign(QString error = "") { this->code = code; this->error = error; return *this; }
  SvException& assign(ErrorType type, QString error = "") { this->type = type; this->code = code; this->error = error; return *this; }

  SvException *clone() const { return new SvException(*this); }


  QString error;
  int code;
  ErrorType type;

};

class SvResult
{
public:

  enum ResultType {
    OK = 0,
    Error = 1
  };

  SvResult(ResultType type = OK, const QString& text = "", int code = 0)
  {
    this->text = text;
    this->type = type;
    this->code = code;
  }

  SvResult& assign(const QString& text) { this->code = code; this->text = text; return *this; }
  SvResult& assign(int code) { this->code = code; return *this; }
  SvResult& assign(ResultType type) { this->type = type; return *this; }

  SvResult *clone() const { return new SvResult(*this); }


  QString text;
  int code;
  ResultType type;

};


#endif // SV_EXCEPTION_H
