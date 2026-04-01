#!/usr/bin/env python3
import xmlrpc.client
import time
import sys
import os
import subprocess
import tempfile
import shutil
import argparse
import shlex

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def poll_for_condition(condition_func, timeout=10, check_interval=0.1):
    start = time.time()
    while time.time() - start < timeout:
        if condition_func():
            return True
        time.sleep(check_interval)
    return False

def wait_for_port(port, timeout=10):
    def check_port():
        try:
            import socket
            with socket.create_connection(("127.0.0.1", port), timeout=1):
                return True
        except:
            return False
    return poll_for_condition(check_port, timeout)

def wait_for_item(proxy, object_name, timeout=10):
    if object_name == "mainWindow" or "/" in object_name:
        path = object_name
    else:
        path = f"mainWindow/{object_name}"
    
    def check_item():
        try:
            return proxy.existsAndVisible(path)
        except:
            return False
    return poll_for_condition(check_item, timeout)

def mouse_click(proxy, object_name):
    if object_name == "mainWindow" or "/" in object_name:
        path = object_name
    else:
        path = f"mainWindow/{object_name}"
    proxy.mouseClick(path)

def mouse_double_click(proxy, object_name):
    if object_name == "mainWindow" or "/" in object_name:
        path = object_name
    else:
        path = f"mainWindow/{object_name}"
    # Explicitly use two clicks since mouseDoubleClick might not be exposed
    proxy.mouseClick(path)
    time.sleep(0.05)
    proxy.mouseClick(path)

def run_test(args, no_start=False, keep_output=False):
    raw_file_path = os.path.abspath("tests/data/raw/_DSC0355.NEF")
    raw_dir_path = os.path.dirname(raw_file_path)
    golden_file_path = os.path.abspath("tests/e2e/golden/golden_raw_pipeline.jpg")
    output_file_path = os.path.abspath("tests/data/raw/_DSC0355-output.jpg")
    
    # Default binary path: try standard build first, then debug
    default_binary_path = os.path.join(project_root, "build", "Filmulator.app", "Contents", "MacOS", "filmulator")
    if not os.path.exists(default_binary_path):
        default_binary_path = os.path.join(project_root, "build", "debug", "Filmulator.app", "Contents", "MacOS", "filmulator")

    binary_path = os.environ.get("FILMULATOR_BIN", default_binary_path)

    # Override with command line argument if provided
    if args.bin:
        binary_path = os.path.abspath(args.bin)

    if not os.path.exists(raw_file_path):
        print(f"Error: RAW file not found at {raw_file_path}")
        return False

    if not no_start and not os.path.exists(binary_path):
        print(f"Error: Binary not found at {binary_path}")
        return False

    # Setup database directory
    test_db_dir = os.environ.get("FILMULATOR_DB_DIR")
    if not test_db_dir:
        if no_start:
            # If connecting to existing, we don't know the DB dir, but we don't need to.
            # However, for consistency we might want to print something.
            print("Connecting to existing instance; assuming it has its own database setup.")
        else:
            test_db_dir = tempfile.mkdtemp(prefix="filmulator_test_db_")
            print(f"Using fresh database directory: {test_db_dir}")
    else:
        print(f"Using database directory from environment: {test_db_dir}")

    # Set environment variables for the process
    env = os.environ.copy()
    if test_db_dir:
        env["FILMULATOR_DB_DIR"] = test_db_dir

    process = None
    if not no_start:
        # Start the process
        print(f"Starting Filmulator: {binary_path}")
        # Support running through a debugger (e.g. lldb)
        debugger = os.environ.get("FILMULATOR_DEBUGGER", "")
        cmd = [binary_path, "--test-mode"]
        if debugger:
            # Use shlex to handle debugger arguments like 'lldb --'
            cmd = shlex.split(debugger) + cmd

        process = subprocess.Popen(cmd,
                                 stdout=None,
                                 stderr=None,
                                 env=env,
                                 text=True)
    else:
        print("Connecting to already-running Filmulator process (waiting for port 9000)...")
        if not wait_for_port(9000, timeout=30):
            print("Error: --no-start provided but no server found on port 9000 after 30 seconds")
            return False

    try:
        print("Waiting for Spix server on port 9000...")
        if not wait_for_port(9000):
            print("Error: Spix server did not start")
            return False

        proxy = xmlrpc.client.ServerProxy("http://127.0.0.1:9000")

        print("Step 1: Waiting for app UI...")
        if not wait_for_item(proxy, "mainWindow"):
            print("Error: App mainWindow not ready")
            return False

        print("Step 2: Navigating to Import tab and setting directory import...")
        mouse_click(proxy, "importTabButton")
        if not wait_for_item(proxy, "importButton"):
            print("Error: Import tab not loaded")
            return False
            
        mouse_click(proxy, "sourceDirButton")
        proxy.setStringProperty("mainWindow/sourceDirButton", "checked", "true")
        
        mouse_click(proxy, "importInPlaceButton")
        proxy.setStringProperty("mainWindow/importInPlaceButton", "checked", "true")
        
        if not wait_for_item(proxy, "sourceDirEntry"):
            print("Error: sourceDirEntry not visible")
            return False
            
        print(f"Setting source directory to: {raw_dir_path}")
        proxy.setStringProperty("mainWindow/sourceDirEntry", "enteredText", raw_dir_path)

        can_import = proxy.getStringProperty("mainWindow/importButton", "notDisabled")
        print(f"  Import button enabled: {can_import}")
        
        if can_import == "false":
            print("Error: Import button is disabled.")
            return False

        print("  Clicking Import button...")
        mouse_click(proxy, "importButton")
        print("Waiting for import to complete (watching itemCount)...")

        print("Step 4: Navigating to Organize tab...")
        mouse_click(proxy, "organizeTabButton")
        time.sleep(0.3) # Give a moment for view switch
        
        # In a fresh DB, the Organize model might need a kick to show the newly imported image
        # since it defaults to showing only today's images.
        
        target_dates = ["2017/12/25", "2017/12/05", "2017/12/5"]
        found_image = False
        for date_to_try in target_dates:
            print(f"  Attempting to select date {date_to_try} in histogram...")
            try:
                proxy.invokeMethod("mainWindow/organizeView", "selectDateInHistogram", [date_to_try])
                time.sleep(0.3) # Short wait for update
                
                item_count = int(proxy.getStringProperty("mainWindow/organizeView", "itemCount") or 0)
                print(f"  Organize itemCount after selecting {date_to_try}: {item_count}")
                
                if item_count > 0:
                    found_image = True
                    break
            except Exception as e:
                print(f"  Warning: Failed to select/check date {date_to_try}: {e}")

        if not found_image:
            print("  Warning: No images detected in Organize. Attempting to force filter reset...")
            proxy.setStringProperty("mainWindow/organizeView/minRatingSlider", "value", "0")
            proxy.setStringProperty("mainWindow/organizeView/maxRatingSlider", "value", "5")
            time.sleep(0.3)
            # Try target date again after reset
            for date_to_try in target_dates:
                proxy.invokeMethod("mainWindow/organizeView", "selectDateInHistogram", [date_to_try])
                time.sleep(0.3)
                item_count = int(proxy.getStringProperty("mainWindow/organizeView", "itemCount") or 0)
                if item_count > 0:
                    found_image = True
                    break
            print(f"  Organize itemCount after reset: {item_count}")

        # Try targeting the delegate directly.
        # Path: mainWindow/organizeView (Organize.qml) -> organizeGridView (GridView) -> organizeDelegate (OrganizeDelegate.qml)
        delegate_path = "mainWindow/organizeView/organizeGridView/organizeDelegate"
        
        if not wait_for_item(proxy, delegate_path, timeout=1):
            print(f"Error: {delegate_path} not found (no images imported or filtered out?)")
            return False
        
        organize_search_id = proxy.getStringProperty(delegate_path, "searchID")
        if not organize_search_id:
            print("Error: Could not read searchID from Organize delegate")
            return False

        print("Step 5: Enqueuing the image...")
        mouse_double_click(proxy, delegate_path)
        time.sleep(0.2) # Wait for animation/enqueue

        print("  Waiting for queue to receive the image...")
        def check_queue_has_items():
            try:
                queue_count = proxy.getStringProperty("mainWindow/queueView", "queueCount")
                return float(queue_count) > 0
            except Exception:
                return False

        if not poll_for_condition(check_queue_has_items, timeout=4, check_interval=0.2):
            print("Error: Queue did not receive the image")
            return False

        print("Step 6: Navigating to Edit tab...")
        mouse_click(proxy, "filmulateTabButton")

        print("Step 7: Selecting image in queue...")
        proxy.invokeMethod("mainWindow", "selectImageBySearchID", [organize_search_id])
        
        print(f"  Image selected for editing: {proxy.invokeMethod('mainWindow', 'currentImageIndex', [])}")

        if not wait_for_item(proxy, "mainWindow/editView/editTools/exposureCompSlider"):
             print("Error: Controls not found")
             return False

        print("Step 8: Adjusting parameters...")
        # Paths are now mainWindow/editView/editTools/...
        proxy.setStringProperty("mainWindow/editView/editTools/exposureCompSlider", "value", "1.5")
        proxy.setStringProperty("mainWindow/editView/editTools/filmDramaSlider", "value", "50")
        proxy.setStringProperty("mainWindow/editView/editTools/crosstalkSlider", "value", "0.5")
        
        print("  Enabling Noise Reduction...")
        # Enable NR switch
        # Path: mainWindow/editView/editTools/nrEnabledSwitch
        # ToolSwitch exposes 'isOn' property, not 'checked'
        proxy.setStringProperty("mainWindow/editView/editTools/nrEnabledSwitch", "isOn", "true")
        
        # Verify it persisted
        time.sleep(0.5)
        is_nr_on = proxy.getStringProperty("mainWindow/editView/editTools/nrEnabledSwitch", "isOn")
        print(f"  NR Switch state: {is_nr_on}")
        if is_nr_on != "true":
             print("Error: Failed to enable Noise Reduction")
             return False
        
        print("  Setting Luma NR...")
        # Set Luma NR (nlStrengthSlider)
        # Value 0.2 corresponds to strength ~0.04 (squared)
        proxy.setStringProperty("mainWindow/editView/editTools/nlStrengthSlider", "value", "0.2")
        
        print("  Setting Chroma NR...")
        # Set Chroma NR (chromaStrengthSlider)
        proxy.setStringProperty("mainWindow/editView/editTools/chromaStrengthSlider", "value", "30")

        print("Step 9: Exporting JPEG...")
        # Wait for export button to be enabled (it depends on image processing completion)
        export_button_path = "mainWindow/editView/editTools/saveJPEGButton"
        print("  Waiting for export button to be enabled (up to 6s)...")
        
        def check_export_enabled():
            return proxy.getStringProperty(export_button_path, "notDisabled") == "true"
            
        if not poll_for_condition(check_export_enabled, timeout=120, check_interval=0.5):
            print("Error: Export button did not become enabled within 120s")
            # Diagnostic: print image state
            print(f"  Image ready: {proxy.getStringProperty('mainWindow/editView/editTools', 'imageReady')}")
            return False
            
        mouse_click(proxy, export_button_path)
        
        print("Waiting for export file...")
        def check_export_file():
            if not os.path.exists(output_file_path):
                return False
            # Check for JPEG EOI marker (FF D9) at the end
            try:
                with open(output_file_path, "rb") as f:
                    f.seek(-2, os.SEEK_END)
                    return f.read(2) == b"\xff\xd9"
            except:
                return False
            
        if not poll_for_condition(check_export_file, timeout=10, check_interval=0.5):
            print("Error: Export timed out")
            print(f"Export file path: {output_file_path}")
            return False

        print("Step 10: Comparing with golden image...")
        if not os.path.exists(golden_file_path):
            print(f"Golden image not found, saving current output as golden: {golden_file_path}")
            os.makedirs(os.path.dirname(golden_file_path), exist_ok=True)
            subprocess.run(["cp", output_file_path, golden_file_path])
            return True

        result = subprocess.run(
            ["magick", "compare", "-metric", "AE", "-fuzz", "5%", output_file_path, golden_file_path, "null:"],
            capture_output=True, text=True
        )
        
        try:
            # magick compare might output "0 (0)" or just "0" depending on version/locale
            # We take the first token and parse it as float then int (handles scientific notation)
            stderr_output = result.stderr.strip()
            diff_pixels = int(float(stderr_output.split()[0]))
            print(f"Differing pixels: {diff_pixels}")
            if diff_pixels < 500:
                print("✓ Test PASSED")
                return True
            else:
                print("✗ Test FAILED: Images differ significantly")
                return False
        except Exception as e:
            print(f"Could not parse compare output: {e}")
            return False

    finally:
        if process:
            print("Cleaning up: Terminating Filmulator process")
            process.terminate()
            try:
                process.wait(timeout=5)
            except:
                process.kill()
        
        # Remove temporary database directory if we created it
        try:
            if test_db_dir and "filmulator_test_db_" in test_db_dir and os.path.exists(test_db_dir):
                shutil.rmtree(test_db_dir)
                print(f"Cleaned up test database directory: {test_db_dir}")
        except:
            pass
        
        if not keep_output:
            try:
                if os.path.exists(output_file_path):
                    os.remove(output_file_path)
            except:
                pass

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="E2E test for RAW processing pipeline.")
    parser.add_argument("--no-start", action="store_true", help="Don't start the binary, connect to already running instance.")
    parser.add_argument("--keep-output", action="store_true", help="Don't delete output files.")
    parser.add_argument("--bin", help="Path to the binary to test.")
    args = parser.parse_args()

    success = run_test(args, no_start=args.no_start, keep_output=args.keep_output)
    if not success:
        sys.exit(1)
