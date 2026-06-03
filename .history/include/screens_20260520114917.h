/**
 * List of available screens / states.
 * 
 * Feel free to extend the list with additional screens, if you decide to implement additional features.
 */
enum screen {
    INIT_SCR,
    ALARM_SCR,
    CLOCK_SCR,
    FACTORY_RESET_SCR,
    SHOW_DATE_SCR,
    SHOW_ENV_SCR
};


/**
 * Individual implementation of screens.
 */
enum screen init_screen(struct context *ctx);
enum screen clock_screen(struct context *ctx);
enum screen factory_reset_screen(struct context *ctx);
enum screen alarm_screen(struct context *ctx);
enum screen show_date_screen(struct context *ctx);
enum screen show_env_screen(struct context *ctx);
