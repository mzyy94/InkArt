<script lang="ts">
  import { onMount } from "svelte";
  import Container from "../templates/Container.svelte";
  import api from "../../api";

  const menuItems = [
    {
      id: "upload",
      value: "#/upload",
      text: "Upload photo",
      icon: "photo",
      subheading: "Edit and upload photo",
    },
    {
      id: "files",
      value: "#/files",
      text: "File Management",
      icon: "folder",
      subheading: "Organize photos to be shown",
    },
    {
      id: "control",
      value: "#/control",
      text: "Display Control",
      icon: "gamepad",
      subheading: "Set display orientation and paddings",
    },
    {
      id: "settings",
      value: "#/settings",
      text: "Settings",
      icon: "settings",
      subheading: "Change device Date/Time, refresh interval, etc.",
    },
    {
      id: "info",
      value: "#/info",
      text: "Information",
      icon: "info",
      subheading: "App version and device information",
    },
    {
      id: "power",
      value: "#",
      action: () => {
        confirmReboot = true;
      },
      text: "Power",
      icon: "power_settings_new",
      subheading: "Reboot and enter photo display mode",
    },
  ];

  onMount(() => {
    if (!("initialized" in window)) {
      //@ts-expect-error
      window["initialized"] = true;
      api.config().then(({ time, refresh }) => {
        if (new Date(time).getFullYear() < 2020) {
          // Sync time
          setTimeout(() => {
            api.config({ time: Date.now(), refresh });
          }, 100);
        }
      });
    }
  });

  let confirmReboot = false;

  function reboot() {
    fetch("/api/v1/system/reboot", { method: "POST" }).then(() => {
      confirmReboot = false;
    });
  }
</script>

<main>
  <Container>
    <span slot="title">InkArt</span>
    <section class="container">
      {#each menuItems as item}
        <a href={item.value} on:click={item.action}>
          <i class="material-icons">{item.icon}</i>
          <div>
            <h4>{item.text}</h4>
            <p>{item.subheading}</p>
          </div>
        </a>
      {/each}
    </section>
  </Container>
</main>

<dialog open={confirmReboot}>
  <article>
    <header>Reboot</header>
    <p>Do you want to leave setup mode?</p>
    <footer>
      <button class="secondary" on:click={() => (confirmReboot = false)}>
        Cancel
      </button>
      <button class="contrast" on:click={reboot}>Reboot</button>
    </footer>
  </article>
</dialog>

<style>
  section > a {
    display: flex;
    align-items: center;
    text-decoration: none;
  }
  section > a:hover {
    background-color: rgba(255, 255, 255, 0.1);
  }
  section > a > i {
    width: 1em;
    margin: 1em;
  }
  section > a > div > * {
    margin: 0;
  }
</style>
