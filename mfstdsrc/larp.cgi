#!/bin/sh

source /etc/init.d/service_registration_functions.sh

SERVICE_NAME="aj_notify"
SERVICE_DEFAULT_HANDLER="/etc/init.d/service_${SERVICE_NAME}.sh"
SERVICE_CUSTOM_EVENTS="\
                        lan_device_detected|$SERVICE_DEFAULT_HANDLER|NULL|$TUPLE_FLAG_EVENT \
                      "
do_stop() {
   sm_unregister $SERVICE_NAME   
}

do_start () {
   sm_register $SERVICE_NAME $SERVICE_DEFAULT_HANDLER "$SERVICE_CUSTOM_EVENTS" 
}

if [ "`syscfg get aj_notify::watchnet`" == "" ] ; then
	syscfg set aj_notify::watchnet 1
fi

echo "Content-Type: text/html"
echo ""

get_cgi_val () {
  if [ "$1" == "" ] ; then
    echo ""
    return
  fi
  form_var="$1"
  var_value=`echo "$QUERY_STRING" | sed -n "s/^.*$form_var=\([^&]*\).*$/\1/p" | sed "s/%20/ /g" | sed "s/+/ /g" | sed "s/%2F/\//g"`
  echo -n "$var_value"
}

echo "<HTML><HEAD>"
echo "<script type='text/javascript'>" 

echo "function doUpdate(){"
echo "cmd = document.getElementById(\"CMD\");" 
echo "cmd.value = \"update\";"
echo "document.getElementById(\"larp\").submit();" 
echo "}"

echo "</script>"
echo "</HEAD><BODY>"

CMD=$(get_cgi_val "CMD")
if [ "$CMD" == "update" ] ; then
	msg=$(get_cgi_val "msg")
	if [ "$msg" ] ; then
		/usr/sbin/aj_notify -n "`hostname`" -l "$msg" -m "Linksys" -a "LarpCGINotify" -d "`syscfg get device::model_base`"
	fi
	enabled=$(get_cgi_val "enabled")
	if [ "$enabled" == "on" ] ; then
		syscfg set alljoyn::enabled 1
	else 
		syscfg set alljoyn::enabled 0
	fi
	watchnet=$(get_cgi_val "watchnet")
	if [ "$watchnet" == "on" ] ; then
		if [ "`syscfg get aj_notify::watchnet`" != "1" ] ; then
			syscfg set aj_notify::watchnet 1
			do_start
		fi
	else
		if [ "`syscfg get aj_notify::watchnet`" != "0" ] ; then
			syscfg set aj_notify::watchnet 0
			do_stop
		fi
	fi
	
	
fi
	echo "<form id=\"larp\" action=\"larp.cgi\">"
	echo "<input type=\"hidden\" id=CMD name=\"CMD\">"
	echo "<p>"
	echo "<h3>Alljoyn Notifications</h3>"
	if [ "`syscfg get alljoyn::enabled`" == "1" ] ; then
		enabled="checked=true"
	else
		enabled=""
	fi
	if [ "`syscfg get aj_notify::watchnet`" == "1" ] ; then
		watchnet="checked=true"
	else
		watchnet=""
	fi
	echo "Alljoyn enabled <input type=\"checkbox\" name=\"enabled\" id=\"enabled\" $enabled><br>"
	echo "Anounce new network devices <input type=\"checkbox\" name=\"watchnet\" id=\"watchnet\" $watchnet><br>"
	echo "Message:<input type=\"text\" name=\"msg\" id=\"msg\" size=\"32\">"
	echo "<br><input type=\"button\" value=\"Update\" onclick=\"doUpdate();\">"
	echo "</p>"
	echo "</form>"



echo "</BODY></HTML>"

