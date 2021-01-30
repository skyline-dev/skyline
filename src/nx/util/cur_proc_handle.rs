use crate::nx::kern::types::{Handle};
use crate::nx::kern::svc;
use crate::nx::arm::register;

use std::thread;

lazy_static! {
    static ref HANDLE : Handle  = get_proc_handle();
}

fn get_proc_handle() -> Handle {

    /* Open up an IPC session to talk to ourselves. */
    let session = svc::create_session(false, 0).unwrap();
    /* Spawn a thread to act as the child for the session. */
    let child = thread::spawn( move || {
        unsafe {
            /* Clear our TLS. */
            let client_tls = std::slice::from_raw_parts_mut(register::get_tls() as *mut u32, 0x10/4);
            client_tls.fill(0);

            svc::output_debug_string("test").unwrap();

            /* Receive IPC message from server. */
            let mut handle_idx: i32 = 0;
            let handles = vec!(session.server);
            svc::reply_and_receive(&mut handle_idx, handles, Handle::INVALID, u64::MAX).unwrap();
            
            /* We're done with the session. */
            let _ = svc::close_handle(session.server);     

            let proc = client_tls[3] as u32;

            svc::output_debug_string("Current process handle: ").unwrap();
            svc::output_debug_string(&proc.to_string()).unwrap();

            /* Get the server process handle from the TLS. */
            Handle(client_tls[3] as u32)
        }
    });

    /* Copy our message to our TLS. */
    let message = [ 0x00000000u32, 0x80000000u32, 0x00000002u32, Handle::CURR_PROC.into() ];
    unsafe {
        let server_tls = std::slice::from_raw_parts_mut(register::get_tls() as *mut u32, 0x10/4);
        &server_tls[..message.len()].copy_from_slice(&message);
    }

    /* Send message to client. */
    let _ = svc::send_sync_request(session.client);

    /* We're done with the session. */
    let _ = svc::close_handle(session.client).unwrap();

    /* Wait for child thread to complete and have it return the handle. */
    child.join().unwrap()
}

pub fn get() -> Handle {
    *HANDLE
}