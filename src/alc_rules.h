#ifndef _ALC_ALC_RULES_H_
#define _ALC_ALC_RULES_H_

#include "log_file.h"
#include "regex.h"
#include "list.h"

typedef struct _AlcRules  AlcRules;

typedef enum _RuleType
{
  RULE_HEAD,
  RULE_BODY,
  RULE_FOOT,
} RuleType;

typedef enum _RuleLogic
{
  RULE_LOGIC_AND = 0,
  RULE_LOGIC_OR,
  RULE_LOGIC_NOT,
} RuleLogic;

struct _AlcRules
{
  RuleLogic logic;
  RuleType  type;
  RegexList rules;
};


AlcRules *alc_rules_new          ();
void      alc_rules_clear        (AlcRules *alc_rules);
int       alc_rules_regex_add    (AlcRules *alc_rules, Regex* regex);
void      alc_rules_logical_set  (AlcRules *alc_rules, RuleLogic logical);
void      alc_rules_type_set     (AlcRules *alc_rules, RuleType type);
int       alc_rules_match_rules  (AlcRules *alc_rules, LogFile *log_file, FILE *stream);

#endif /* _ALC_ALC_RULES_H_ */
