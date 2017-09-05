#include "lexer.h"

const char *strtoken(int token) {
  switch (token) {
  	case TOK_ERROR:		      return "TOK_ERROR";
  	case TOK_NO_TOK:		    return "TOK_NO_TOK";
  	case TOK_VARID:		      return "TOK_VARID";
  	case TOK_CONID:		      return "TOK_CONID";
  	case TOK_NUMBER:		    return "TOK_NUMBER";
    case TOK_OP:		        return "TOK_OP";
    case TOK_NEWLINE:       return "TOK_NEWLINE";
  	case TOK_CASE:    	  	return "TOK_CASE";
  	case TOK_CLASS:   	  	return "TOK_CLASS";
  	case TOK_DATA:	      	return "TOK_DATA";
  	case TOK_DEFAULT: 	  	return "TOK_DEFAULT";
  	case TOK_DERIVING:	  	return "TOK_DERIVING";
  	case TOK_DO:		        return "TOK_DO";
  	case TOK_ELSE:	      	return "TOK_ELSE";
  	case TOK_FOREIGN:	    	return "TOK_FOREIGN";
  	case TOK_IF:		        return "TOK_IF";
  	case TOK_IMPORT:		    return "TOK_IMPORT";
  	case TOK_IN:		        return "TOK_IN";
  	case TOK_INFIX:   	  	return "TOK_INFIX";
  	case TOK_INFIXL:  	  	return "TOK_INFIXL";
  	case TOK_INFIXR:  		  return "TOK_INFIXR";
  	case TOK_INSTANCE:		  return "TOK_INSTANCE";
  	case TOK_LET:		        return "TOK_LET";
  	case TOK_MODULE:  	  	return "TOK_MODULE";
  	case TOK_NEWTYPE: 	  	return "TOK_NEWTYPE";
  	case TOK_OF:		        return "TOK_OF";
  	case TOK_THEN:    	  	return "TOK_THEN";
  	case TOK_TYPE:		      return "TOK_TYPE";
  	case TOK_WHERE:		      return "TOK_WHERE";
  	case TOK_RANGE:	      	return "TOK_RANGE";
  	case TOK_HASTYPE:	    	return "TOK_HASTYPE";
  	case TOK_L_ARROW:	    	return "TOK_L_ARROW";
  	case TOK_R_ARROW:	    	return "TOK_R_ARROW";
    case TOK_R_FAT_ARROW:		return "TOK_R_FAT_ARROW";
	case '(':		            return "(";
	case ')':		            return ")";
	case ',':		            return ",";
	case ';':		            return ";";
	case '[':		            return "[";
	case ']':		            return "]";
	case '`':		            return "`";
	case '{':		            return "{";
	case '}':		            return "}";
	case '_':		            return "_";
	case '=':		            return "=";
	case '\\':		            return "\\";
	case '|':		            return "|";
	case '@':		            return "@";
	case '~':		            return "~";
	case '.':		            return ".";
  }

  return "TOK_UNKNOWN";
}