#!/bin/sh
echo "Content-Type: text/html"
echo ""

PAGE_TITLE=""

get_cgi_val () {
  if [ "$1" == "" ] ; then
    echo ""
    return
  fi
  form_var="$1"
  var_value=`echo "$QUERY_STRING" | sed -n "s/^.*$form_var=\([^&]*\).*$/\1/p" | sed "s/%20/ /g" | sed "s/+/ /g" | sed "s/%2F/\//g" | sed "s/%2C/,/g"`
  echo -n "$var_value"
}

get_cgi_json () {
  if [ "$1" == "" ] ; then
    echo ""
    return
  fi
  form_var="$1"
  var_value=`echo "$QUERY_STRING" | sed -n "s/^.*$form_var=\([^&]*\).*$/\1/p" | sed "s/%20/ /g" | sed "s/+/ /g" | sed "s/%2F/\//g" | sed "s/%2C/,/g" | sed 's/%22/"/g' | sed "s/%3A/:/g" | sed "s/%0D//g" | sed "s/%0A//g" | sed "s/%7B/{/g" | sed "s/%7D/}/g" | sed "s/%5B/[/g" | sed "s/%5D/]/g"`
  echo -n "$var_value"
}

echo "<HTML><HEAD><TITLE>${PAGE_TITLE}</TITLE>"
echo "<script type='text/javascript'>" 
echo "function doSubmit(){"
echo "cmd = document.getElementById(\"CMD\");" 
echo "cmd.value = \"POST\";"
echo "document.getElementById(\"myform\").submit();" 
echo "}"
echo "</script>"
echo "</HEAD><BODY>"
echo "<h3>${PAGE_TITLE}</h3>"

cmd=$(get_cgi_val "CMD")

echo "<form id=\"myform\" action=\"shell_cgi.cgi\">"
echo "<input type=\"hidden\" id=CMD name=\"CMD\">"
echo "</form>"
echo ""
echo "</BODY></HTML>"

