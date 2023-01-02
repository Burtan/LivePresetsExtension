use reaper_high::Reaper;
use reaper_macros::reaper_extension_plugin;
use std::error::Error;

#[reaper_extension_plugin(name = "Hello World", support_email_address = "support@example.org")]
fn plugin_main() -> Result<(), Box<dyn Error>> {
    let reaper = Reaper::get().medium_reaper();
    reaper.show_console_msg("Hello from reaper-rs!\n");
    Ok(())
}