package tutorial;

import java.text.SimpleDateFormat;
import java.util.Date;

class Function
{
	public static void greet(Date date)
	{
		System.out.printf("Hello World @ %s\r\n", new SimpleDateFormat( "dd-MM-yyyy HH:mm:ss" ).format(date));
	}
}