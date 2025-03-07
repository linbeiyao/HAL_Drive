

  lcd_init();
  lcd_send_string("Hello World!");
  HAL_Delay(1000);
  lcd_put_cur(1, 0);
  lcd_send_string("from Cedr1c");
